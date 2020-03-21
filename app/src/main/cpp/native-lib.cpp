#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdio.h>
#include <iostream>
#include "alog.h"
#include "FileOption.h"
#include "base64.h"
#include "Buffer.h"

using namespace std;

static int registerNativeMethods(JNIEnv *env, jclass cls);
static Buffer *getBuffer(jlong addr);

extern "C" JNIEXPORT JNICALL jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    static const char *clsName = "com/andy/logdog/Logdog";
    jclass instance = env->FindClass(clsName);
    if(!instance) {
        LOGE("fail to locate class: %s", clsName);
        return -2;
    }

    int ret = registerNativeMethods(env, instance);
    if(0 != ret) {
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
    Buffer *buffer = new Buffer();
    buffer->doInit(path);
    env->ReleaseStringUTFChars(jpath, path);
    return (jlong)buffer;
}


static void mmapWrite(JNIEnv *env, jobject thiz,
                      jlong buffer, jstring content) {
    Buffer *bufferStatic = getBuffer(buffer);
    if(nullptr == bufferStatic || !bufferStatic->isInit()) {
        LOGW("buffer not init");
        return;
    }

    const char *content_chars = env->GetStringUTFChars(content, JNI_FALSE);
    LOGD("[mmap]:content: %s", content_chars);

    bool resultAppend = bufferStatic->append(content_chars);
    if(resultAppend) {
        LOGI("[NativeLib] mmap write success");
    } else {
        LOGE("[NativeLib] mmap write fail");
    }

    LOGD("[NativeLib-write] release content");
    env->ReleaseStringUTFChars(content, content_chars);
    LOGD("[NativeLib-write] all done");
}


/**
 * 从指定的buffer中读取内容
 * @param env
 * @param thiz
 * @param buffer
 * @param path
 * @return
 */
static jstring readFile(JNIEnv *env,jobject thiz,
                        jlong buffer) {
    Buffer *bufferStatic = getBuffer(buffer);

    if(nullptr == bufferStatic) {
        return nullptr;
    }

    //read file content to buffer
    char* read = bufferStatic->getAll();
    const char *contentFromFile = read;
    //convert date type
    jstring result = env->NewStringUTF(contentFromFile == nullptr ? "" : contentFromFile);
    //release memory
    if(nullptr != read)
    free((void *) read);

    return result;
}

static void onExit(JNIEnv *env, jobject thiz, jlong buffer) {
    Buffer *bufferStatic = getBuffer(buffer);
    if(nullptr == bufferStatic) return;
    bufferStatic->onExit();
    delete bufferStatic;
}


static JNINativeMethod methods[] = {
        {"createBuffer", "(Ljava/lang/String;)J", (void *)createBuffer},
        {"mmapWrite",  "(JLjava/lang/String;)V", (void *)mmapWrite},
        {"readFile",   "(J)Ljava/lang/String;", (void *)readFile},
        {"onExit",     "(J)V", (void *)onExit}
};

static int registerNativeMethods(JNIEnv *env, jclass cls) {
    return env->RegisterNatives(cls,
            methods,
            sizeof(methods) / sizeof(methods[0]));
}

static Buffer *getBuffer(jlong addr) {
    return (Buffer *)addr;
}