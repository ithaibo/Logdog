#include <jni.h>
#include <cstring>
#include <android/log.h>
#include <cstdio>
#include <iostream>

#include "alog.h"
#include "FileOption.h"
#include "base64.h"
#include "Buffer.h"
#include <unistd.h>
#include "compress/Zip.h"
#include <zlib.h>

#include "log_protocol.h"
#include "config.h"
#include "utils.h"
#include "log_type.h"
#include <vector>

using namespace std;

static int registerNativeMethods(JNIEnv *env, jclass cls);

static Buffer *getBuffer(jlong addr);

extern "C" JNIEXPORT JNICALL jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    static const char *clsName = "com/andy/mmap/Mmap";
    jclass instance = env->FindClass(clsName);
    if (!instance) {
        LOGE("fail to locate class: %s", clsName);
        return -2;
    }

    int ret = registerNativeMethods(env, instance);
    if (0 != ret) {
        LOGE("fail to register native methods");
        return -3;
    }

    return JNI_VERSION_1_6;
}

/**
 * 创建Buffer，并返回其地址
 * @param env
 * @param obj
 * @param jpath
 * @return
 */
static jlong createBuffer(JNIEnv *env, jobject obj, jstring jpath) {
    const char *path = env->GetStringUTFChars(jpath, JNI_FALSE);
    auto *buffer = new Buffer();
    buffer->doInit(path);
    env->ReleaseStringUTFChars(jpath, path);
    return (jlong) buffer;
}

static jboolean mmapWrite(JNIEnv *env, jobject thiz, jlong buffer, jstring content) {
    Buffer *bufferStatic = getBuffer(buffer);
    if (nullptr == bufferStatic || !bufferStatic->isInit()) {
        LOGW("buffer not init");
        return false;
    }

    const char *content_chars = env->GetStringUTFChars(content, JNI_FALSE);
    int length = strlen(content_chars);

    //compress
    string compressedStr;
    int codeCompress = compress((uint8_t *) content_chars, length, compressedStr,
                                Z_BEST_COMPRESSION);
    if(Z_OK != codeCompress) {
        env->ReleaseStringUTFChars(content, content_chars);
        LOGE("[mmap] compress failed");
        return false;
    }
    int lengthAfterCompress = compressedStr.length();
    HbLog log = LogProtocol::createLogItem(compressedStr, lengthAfterCompress);
    printLog(log);

    //serialize
    uint8_t *toSave = LogProtocol::serialize(log);
    //save
    bool resultAppend = bufferStatic->append(toSave, log.logLength);
    //clear up
    delete []toSave;
    env->ReleaseStringUTFChars(content, content_chars);
    return resultAppend;
}


/**
 * 从指定的buffer中读取内容
 * @param env
 * @param thiz
 * @param buffer
 * @param path
 * @return
 */
static jstring readFile(JNIEnv *env, jobject thiz, jlong buffer) {
    Buffer *bufferStatic = getBuffer(buffer);
    if (nullptr == bufferStatic) {
        return nullptr;
    }
    FileOption fileOption;
    string readFromFile = fileOption.readFileAll(bufferStatic->getFilePath());
    if (readFromFile.empty()) {
        return nullptr;
    }
    // 这里可能存在多条日志
    uint8_t off[readFromFile.length()+1];
    memcpy(off, readFromFile.data(), readFromFile.length());
    off[readFromFile.length()] = '\0';
    readFromFile.data();
    const size_t length = readFromFile.length();
    uint8_t *end = off + length;
    string temp;
    vector<LogRegionNeedParse> allMagicPos = LogProtocol::parseAllMagicPosition(readFromFile);
    LOGD("[bridge] count magic position found:%d", allMagicPos.size());
    for_each(allMagicPos.begin(), allMagicPos.end(), [&](LogRegionNeedParse serializedLog) {
        HbLog parsedLog = LogProtocol::parseOneLog(readFromFile, serializedLog);
        if (parsedLog.logLength > 0 && (!parsedLog.body.content.empty())) {
            printLog(parsedLog);
            temp.append(parsedLog.body.content);
            LOGD("[bridge] append body content:%s", parsedLog.body.content.data());
            temp.append("\n");
        }
    });
    LOGD("[bridge] total log parsed count:%d", allMagicPos.size());
    LOGD("[bridge] all read from file, length:%d, content:%s", temp.length(), temp.c_str());
    //release memory
    jstring result = env->NewStringUTF(temp.empty()? "" : temp.c_str());
    //release memory
    return result;
}

static void onExit(JNIEnv *env, jobject thiz, jlong buffer) {
    Buffer *bufferStatic = getBuffer(buffer);
    if (nullptr == bufferStatic) return;
    bufferStatic->onExit();
    delete bufferStatic;
}


static JNINativeMethod methods[] = {
        {"createBuffer", "(Ljava/lang/String;)J",  (void *) createBuffer},
        {"mmapWrite",    "(JLjava/lang/String;)Z", (void *) mmapWrite},
        {"readFile",     "(J)Ljava/lang/String;",  (void *) readFile},
        {"onExit",       "(J)V",                   (void *) onExit}
};

static int registerNativeMethods(JNIEnv *env, jclass cls) {
    return env->RegisterNatives(cls,
                                methods,
                                sizeof(methods) / sizeof(methods[0]));
}

static Buffer *getBuffer(jlong addr) {
    return (Buffer *) addr;
}