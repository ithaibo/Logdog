#include <jni.h>
#include <cstring>
#include <android/log.h>
#include <cstdio>
#include <iostream>

#include "alog.h"
#include "FileOption.h"
#include "Buffer.h"
#include <unistd.h>
#include "compress/Zip.h"
#include <zlib.h>

#include "log_protocol.h"
#include "config.h"
#include "utils.h"
#include "log_type.h"
#include <vector>

#include "MmapMain.h"

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
    Buffer *buffer = MmapMain::createBuffer(path);
    return (jlong) buffer;
}

/**
 * 保存
 * @param env
 * @param thiz
 * @param buffer
 * @param content
 * @return
 */
static jboolean mmapWrite(JNIEnv *env, jobject thiz, jlong buffer, jstring content) {
    const char *content_chars = env->GetStringUTFChars(content, JNI_FALSE);
    unsigned long long timestart = getTimeUSDNow();
    MmapMain::getTrace()->timestamp = timestart;
    bool saveResult;
    saveResult = MmapMain::mmapWrite(getBuffer(buffer), content_chars);

    //todo all done
    MmapMain::getTrace()->flush2File();

    unsigned long long timeend = getTimeUSDNow();
    LOGI("[bridge] time cost 10000 write:%llu", (timeend - timestart));
    env->ReleaseStringUTFChars(content, content_chars);
    return saveResult;
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
    LOGD("[bridge] read file done, string address:%d", &readFromFile);
    if (readFromFile.empty()) {
        return nullptr;
    }
    // 这里可能存在多条日志
    LOGD("[bridge] start to parse magic positions...");
    vector<LogRegionNeedParse> allMagicPos = LogProtocol::parseAllMagicPosition(readFromFile);
    LOGD("[bridge] count magic position found:%d", allMagicPos.size());
    string temp;
    for_each(allMagicPos.begin(), allMagicPos.end(), [&](LogRegionNeedParse serializedLog) {
        HbLog parsedLog = LogProtocol::parseOneLog(readFromFile, serializedLog);
        if (parsedLog.logLength > 0 && !parsedLog.body.content.empty()) {
            printLog(parsedLog);
            temp.append(parsedLog.body.content);
            temp.append("\n");
        }
    });
    LOGD("[bridge] total log parsed count:%d", allMagicPos.size());
    LOGD("[bridge] all read from file, length:%d", temp.length());
    //release memory
    jstring result = env->NewStringUTF(temp.empty()? "" : temp.c_str());
    //release memory
    return result;
}

static void onExit(JNIEnv *env, jobject thiz, jlong buffer) {
    MmapMain::flushAndArchive(getBuffer(buffer));
}


static JNINativeMethod methods[] = {
        {"createBuffer", "(Ljava/lang/String;)J",  (void *) createBuffer},
        {"mmapWrite",    "(JLjava/lang/String;)Z", (void *) mmapWrite},
        {"readFile",     "(J)Ljava/lang/String;",  (void *) readFile},
        {"onExit",       "(J)V",                   (void *) onExit}
};

static int registerNativeMethods(JNIEnv *env, jclass cls) {
    return env->RegisterNatives(cls, methods, sizeof(methods) / sizeof(methods[0]));
}

static Buffer *getBuffer(jlong addr) {
    return (Buffer *) addr;
}