//
// Created by haibo.wu on 2021/1/22.
//

#ifndef LOGDOG_UTILS_H
#define LOGDOG_UTILS_H

#include <sys/time.h>

inline unsigned long getTimeStamp() {
    timeval time;
    gettimeofday(&time, nullptr);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

#endif //LOGDOG_UTILS_H
