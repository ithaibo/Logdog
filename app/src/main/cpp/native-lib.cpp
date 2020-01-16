#include <jni.h>
#include <string>
#include <android/log.h>
#include <stdio.h>
#include <iostream>
#include "alog.h"
#include "logdog.h"

using namespace std;


static void releaseStringUTFChars(JNIEnv *env, jstring path, const char* chars) {
    if(nullptr == env) return;
    (*env).ReleaseStringUTFChars(path, chars);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_native_1init(JNIEnv *env, jobject thiz, jstring logpath) {
    const char *path = (*env).GetStringUTFChars(logpath, JNI_FALSE);

    (*env).ReleaseStringUTFChars(logpath, path);

    // TODO: create file
//    create_file();
    // TODO: mmap
}

extern "C"
JNIEXPORT void JNICALL
Java_com_andy_logdog_Logdog_mmap_1write(JNIEnv *env, jobject thiz, jstring path,
                                          jstring content) {
    const char *path_chars = (*env).GetStringUTFChars(path, JNI_FALSE);
    const char *content_chars = (*env).GetStringUTFChars(content, JNI_FALSE);

    LOGD("[mmap]:path: %s", path_chars);
    LOGD("[mmap]:content: %s", content_chars);

    MmapWrite(path_chars, content_chars);

    (*env).ReleaseStringUTFChars(path, path_chars);
    (*env).ReleaseStringUTFChars(content, content_chars);
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
    const char* contentFromFile = readWithMmap(filePath);
    releaseStringUTFChars(env, path, filePath);
    return (*env).NewStringUTF(contentFromFile== nullptr? "" : contentFromFile);
}

