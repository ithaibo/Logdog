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


static void releaseStringUTFChars(JNIEnv *env, jstring jstr, const char* chars) {
    if(nullptr == env) return;
    (*env).ReleaseStringUTFChars(jstr, chars);
}

static Buffer *bufferStatic = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_nativeInit(JNIEnv *env, jobject thiz, jstring logpath) {
    const char *path = (*env).GetStringUTFChars(logpath, JNI_FALSE);
    bufferStatic = &Buffer::get_instance(path);
    (*env).ReleaseStringUTFChars(logpath, path);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_mmapWrite(JNIEnv *env, jobject thiz,
                                          jstring content) {
    const char *content_chars = (*env).GetStringUTFChars(content, JNI_FALSE);
    LOGD("[mmap]:content: %s", content_chars);
    LOGD("address of buffer: %d", bufferStatic);

    bool resultAppend = bufferStatic->append(content_chars);
    if(resultAppend) {
        LOGI("[NativeLib] mmap write success");
    } else {
        LOGE("[NativeLib] mmap write fail");
    }

    LOGD("[NativeLib-write] release content");
    (*env).ReleaseStringUTFChars(content, content_chars);
    LOGD("[NativeLib-write] all done");
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_andy_logdog_Logdog_readFile(JNIEnv *env, jobject thiz, jstring path) {
    const char* filePath = (*env).GetStringUTFChars(path, JNI_FALSE);
    FileOption *fileOption = new FileOption();
    //read file content to buffer
    const char* contentFromFile = fileOption->readFile(filePath);
    //convert date type
    releaseStringUTFChars(env, path, filePath);
    jstring result = (*env).NewStringUTF(contentFromFile== nullptr? "" : contentFromFile);
    //release memory
    fileOption->freeTempBuffer();
    delete fileOption;

    return result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_printBase64(JNIEnv *env, jobject thiz, jstring content) {
    const char* raw = (*env).GetStringUTFChars(content, JNI_FALSE);
    char* afterBase64 = base64_encode(raw);
    LOGD("[base64] raw: %s, size: %ld", raw, strlen(raw));
    LOGD("[base64] afterBase64: %s, size: %ld", afterBase64, strlen(afterBase64));
    LOGD("[base64] afterBase64Decode: %s", base64_decode(afterBase64));

    releaseStringUTFChars(env, content, raw);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_onExit(JNIEnv *env, jobject thiz) {
    if(nullptr == bufferStatic) return;
    bufferStatic->onExit();
}