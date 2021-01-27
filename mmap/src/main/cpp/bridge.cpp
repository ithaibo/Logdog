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
    shared_ptr<HbLog> log = LogProtocol::createLogItem((uint8_t *) compressedStr.c_str(), lengthAfterCompress);
    printLog(log.get());

    //serialize
    uint8_t *toSave = LogProtocol::serialize(log.get());
    //save
    bool resultAppend = bufferStatic->append(toSave, log->logLength);
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
    auto *off = (uint8_t *) readFromFile.c_str();
    size_t length = readFromFile.length();
    for(int i = length - 1; i >= 0; i--) {
        if (off[i] == '\0') continue;
        length = i + 1;
        break;
    }
    uint8_t *end = off + length;
    vector<shared_ptr<HbLog>> logParseList;
    string temp;
    while (off < end) {
        //pare one log
        shared_ptr<HbLog> parsedLog = LogProtocol::deserialize(off);
        if(nullptr == parsedLog) {//寻找第一个MAGIC
            off++;
            continue;
        }
        uint8_t * bodyContent = !parsedLog->body? nullptr : parsedLog->body->content;
        printLog(parsedLog.get());
        if(parsedLog->logLength <= 0) {
            LOGW("[bridge] log length parse <= 0");
            break;
        }
        logParseList.push_back(parsedLog);
        off += parsedLog->logLength;
        if(!parsedLog -> body || !bodyContent) {
            LOGW("[bridge] log body or content of body is empty");
            continue;
        }
        temp.append((const char*)bodyContent);
        temp.append("\n");
    }
    LOGD("[bridge] total log parsed count:%d", logParseList.size());
    LOGD("[bridge] all read from file, length:%d, content:%s", temp.length(), temp.c_str());
    //release memory
    logParseList.clear();
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