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

static Buffer *bufferStatic = nullptr;
static int registerNativeMethods(JNIEnv *env, jclass cls);

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


static void releaseStringUTFChars(JNIEnv *env, jstring jstr, const char* chars) {
    if(nullptr == env) return;
    env->ReleaseStringUTFChars(jstr, chars);
}

static void nativeInit(JNIEnv *env, jobject obj,
        jstring logpath) {
    const char *path = env->GetStringUTFChars(logpath, JNI_FALSE);
    if(nullptr == bufferStatic) {
        LOGD("create and init buffer...");
        bufferStatic = &Buffer::get_instance(path);
    } else if (!bufferStatic->isInit()) {
        LOGD("buffer was created, init it...");
        bufferStatic->doInit(path);
    } else {
        LOGD("buffer was ready, no need init again.");
    }
    env->ReleaseStringUTFChars(logpath, path);
}

static void mmapWrite(JNIEnv *env, jobject thiz,
                      jstring content) {
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


//todo refactor 指定读取的buffer
static jstring readFile(JNIEnv *env,jobject thiz,
        jstring path) {
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

static void onExit(JNIEnv *env, jobject thiz) {
    if(nullptr == bufferStatic) return;
    bufferStatic->onExit();
}


static JNINativeMethod methods[] = {
        {"nativeInit", "(Ljava/lang/String;)V", (void *)nativeInit},
        {"mmapWrite",  "(Ljava/lang/String;)V", (void *)mmapWrite},
        {"readFile",   "(Ljava/lang/String;)Ljava/lang/String;", (void *)readFile},
        {"onExit",     "()V", (void *)onExit}
};

static int registerNativeMethods(JNIEnv *env, jclass cls) {
    return env->RegisterNatives(cls,
            methods,
            sizeof(methods) / sizeof(methods[0]));
}