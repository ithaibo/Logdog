//
// Created by haibo.wu on 2021/1/22.
//

#ifndef LOGDOG_UTILS_H
#define LOGDOG_UTILS_H

#include <sys/time.h>
#include "alog.h"
#include <time.h>

inline uint64_t getTimeStamp() {
    timeval time;
    gettimeofday(&time, nullptr);
    uint64_t timestamp = (uint64_t)time.tv_sec * 1000 + time.tv_usec / 1000;
    return timestamp;
}

inline unsigned long long getTimeUSDNow() {
    timeval time;
    struct timezone tz;
    gettimeofday(&time, &tz);
    LOGD("[utils] tv.sec:%ld, tv.usec:%ld", time.tv_sec, time.tv_usec);
    unsigned long long timestamp =  time.tv_usec;
    timestamp += time.tv_sec * 1000000;

    return timestamp;
}

inline char *uint32ToChars(uint32_t input) {
    char *result = new char[4];
    memset(result, '\0', 4);
    for (int i = 0; i < 4; i++) {
        result[3 - i] = (input >> (i * 8));
    }
    return result;
}

inline bool cpIntToByteArray(uint32_t &intVal, uint8_t *byteArr) {
    if (!byteArr) {
        LOGW("[utils] byteArr is null");
        return false;
    }
    byteArr[0] = (uint8_t) (0xff & intVal);
    byteArr[1] = (uint8_t) ((0xff00 & intVal) >> 8);
    byteArr[2] = (uint8_t) ((0xff0000 & intVal) >> 16);
    byteArr[3] = (uint8_t) ((0xff000000 & intVal) >> 24);
    return true;
}

inline uint32_t charsToUint32(const char *byteArr) {
    if (!byteArr) return -1;
    int result = 0;

    result = byteArr[0] & 0xFF;
    result |= ((byteArr[1] << 8) & 0xFF00);
    result |= ((byteArr[2] << 16) & 0xFF0000);
    result |= ((byteArr[3] << 24) & 0xFF000000);
    return result;
}

#endif //LOGDOG_UTILS_H
