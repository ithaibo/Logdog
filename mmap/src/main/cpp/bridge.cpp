#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdio.h>
#include <iostream>

#include "alog.h"
#include "FileOption.h"
#include "base64.h"
#include "Buffer.h"
#include <unistd.h>
#include "compress/Zip.h"

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
    int pid = getpid();
    long tid = pthread_self();
//    LOGD("process id: %d, thread id:%ld mmapWrite invoked", pid, tid);
    Buffer *bufferStatic = getBuffer(buffer);
    if(nullptr == bufferStatic || !bufferStatic->isInit()) {
        LOGW("buffer not init");
        return;
    }

    const char *content_chars = env->GetStringUTFChars(content, JNI_FALSE);
//    LOGD("[mmap]:content: %s", content_chars);
    int length = strlen(content_chars);
    LOGD("[mmap] before compress length:%d", length);
    std::string out;
    int codeCompress = compress(content_chars, length, out, Z_DEFAULT_COMPRESSION);
    LOGD("[mmap] out.length():%d, strlen(out.c_str):%d", out.length(), strlen(out.c_str()));
    std::string raw;
    int code2 = decompress(out.c_str(), out.length(), raw);
    if (Z_OK == code2 && codeCompress == Z_OK) {
        LOGD("[mmap] compare compress and decompress:%d", strcmp(content_chars, raw.c_str()));
    }
    int lengthAfterCompress = out.length();
    if (Z_OK == codeCompress) {
        LOGD("[mmap] after compress, code compress:%d, length:%d", codeCompress, lengthAfterCompress);
        bool resultAppend = bufferStatic->append(reinterpret_cast<const uint8_t *>(out.c_str()), out.length());
        if(resultAppend) {
            LOGI("[NativeLib] mmap write success");
        } else {
            LOGE("[NativeLib] mmap write fail");
        }
    } else {
        LOGE("[mmap] compress failed");
    }
    env->ReleaseStringUTFChars(content, content_chars);
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
    std::string * compressed = bufferStatic->getAll();
    if (nullptr == compressed) {
        return nullptr;
    }
    std::string decompressResult;
    if (!compressed->empty()) {
        LOGD("[mmap] to decompress, length:%d", compressed->length());
        int codeDecompress = decompress(compressed->c_str(), compressed->length(), decompressResult);
        int lengthDecompress = decompressResult.length();
        LOGD("[mmap] after decompress, code:%d, length:%d",codeDecompress, lengthDecompress);
        LOGD("[mmap] content decompress%s",decompressResult.c_str());
    }
    //convert date type
    jstring result = env->NewStringUTF(decompressResult.empty() ? "" : decompressResult.c_str());
    //release memory
    delete compressed;

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