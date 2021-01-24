//
// Created by haibo.wu on 2021/1/22.
//

#ifndef LOGDOG_UTILS_H
#define LOGDOG_UTILS_H

#include <sys/time.h>
#include "alog.h"
#include <time.h>

inline unsigned long getTimeStamp() {
    timeval time;
    gettimeofday(&time, nullptr);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

inline unsigned long long getTimeUSDNow() {
    timeval time;
    struct timezone tz;
    gettimeofday(&time, &tz);
    LOGD("[utils] tv.sec:%d, tv.usec:%d", time.tv_sec, time.tv_usec);
    unsigned long long timestamp =  time.tv_usec;
    timestamp += time.tv_sec * 1000000;

    return timestamp;
}

#endif //LOGDOG_UTILS_H
