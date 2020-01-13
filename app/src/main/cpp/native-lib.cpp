#include <jni.h>
#include <string>
#include <android/log.h>
#include "alog.h"
#include <stdio.h>

using namespace std;

extern "C" JNIEXPORT jstring JNICALL
Java_com_andy_logdog_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

char *log_path = NULL;


FILE *fp = NULL;

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
Java_com_andy_logdog_Logdog_native_1write(JNIEnv *env, jobject thiz, jstring tag,
                                          jstring content) {
    const char *tag_chars = (*env).GetStringUTFChars(tag, JNI_FALSE);
    const char *content_chars = (*env).GetStringUTFChars(content, JNI_FALSE);

//    __android_log_print(ANDROID_LOG_DEBUG, "logdog", printf("%s\n", tag_chars));
//    __android_log_print(ANDROID_LOG_DEBUG, "logdog", "%s\n", content_str);

    LOGD("[%s]:\t%s", tag_chars, content_chars);
    LOGD("%s\n", content_chars);
    (*env).ReleaseStringUTFChars(tag, tag_chars);
    (*env).ReleaseStringUTFChars(content, content_chars);
    // TODO: implement native_write()

}