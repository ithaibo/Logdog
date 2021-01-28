//
// Created by haibo.wu on 2021/1/13.
//

#ifndef LOGDOG_TIMETRACE_H
#define LOGDOG_TIMETRACE_H


class TimeTrace {
private:
    int times;
    long long initCost;
    long long openFile;
    long long mmap;
};


#endif //LOGDOG_TIMETRACE_H
