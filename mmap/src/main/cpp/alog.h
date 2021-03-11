//
// Created by haibo.wu on 2020/1/10.
//

#ifndef LOGDOG_ALOG_H
#define LOGDOG_ALOG_H

#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "NATIVELOG"
#endif

#define DEBUG
#define ANDROID_PLATFORM

#ifdef DEBUG
#ifdef ANDROID_PLATFORM
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))


#else
#define LOGD(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define LOGI(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define LOGW(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)
#define LOGE(fmt, ...) printf(fmt"\n", ##__VA_ARGS__)

#endif
#else
#define LOGD(...);
	#define LOGI(...);
	#define LOGW(...);
	#define LOGE(...);
#endif

#endif //LOGDOG_ALOG_H
