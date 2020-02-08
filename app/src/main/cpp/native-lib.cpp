#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdio.h>
#include <iostream>
#include "alog.h"
#include "logdog.h"
#include "base64.h"
#include "Buffer.h"

using namespace std;


static void releaseStringUTFChars(JNIEnv *env, jstring jstr, const char* chars) {
    if(nullptr == env) return;
    (*env).ReleaseStringUTFChars(jstr, chars);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_native_1init(JNIEnv *env, jobject thiz, jstring logpath) {
    const char *path = (*env).GetStringUTFChars(logpath, JNI_FALSE);

//    Buffer::get_instance().mapMemory(path);

    (*env).ReleaseStringUTFChars(logpath, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_mmap_1write(JNIEnv *env, jobject thiz, jstring path,
                                          jstring content) {
    const char *path_chars = (*env).GetStringUTFChars(path, JNI_FALSE);
    const char *content_chars = (*env).GetStringUTFChars(content, JNI_FALSE);
    LOGD("[mmap]:path: %s", path_chars);
    LOGD("[mmap]:content: %s", content_chars);

    Buffer buffer = Buffer::get_instance(path_chars);
    LOGD("address of buffer: %d", &buffer);
//    buffer.setFilePath(path_chars);

    bool resultAppend = buffer.append(path_chars, content_chars);
    if(resultAppend) {
        LOGI("[NativeLib] mmap write success");
    } else {
        LOGE("[NativeLib] mmap write fail");
    }
//    buffer.setFilePath(nullptr);

    LOGD("[NativeLib-write] release path");
    (*env).ReleaseStringUTFChars(path, path_chars);
    LOGD("[NativeLib-write] release content");
    (*env).ReleaseStringUTFChars(content, content_chars);

    LOGD("[NativeLib-write] all done");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_write_1file(JNIEnv *env, jobject thiz, jstring path, jstring content) {
    const char* filePath = (*env).GetStringUTFChars(path, JNI_FALSE);
    const char* contentSave = (*env).GetStringUTFChars(content, JNI_FALSE);

    writeFile(filePath, contentSave);

    releaseStringUTFChars(env, path, filePath);
    releaseStringUTFChars(env, content, contentSave);
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_andy_logdog_Logdog_read_1file(JNIEnv *env, jobject thiz, jstring path) {
    const char* filePath = (*env).GetStringUTFChars(path, JNI_FALSE);
    const char* contentFromFile = readFile(filePath);
    releaseStringUTFChars(env, path, filePath);
    return (*env).NewStringUTF(contentFromFile== nullptr? "" : contentFromFile);
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
