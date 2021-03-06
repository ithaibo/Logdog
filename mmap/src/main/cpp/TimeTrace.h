//
// Created by haibo.wu on 2021/1/13.
//

#ifndef LOGDOG_TIMETRACE_H
#define LOGDOG_TIMETRACE_H

#include <vector>

namespace LogTrace {

    enum ActionId {
        zip = 0,
        encrypt,
        protocol,
        serialize,
        unmap,
        TRUNCATE,
        map,
        saveBuffer
    };

    template<class T, class T2>
    class Pair {
    public:
        T first;
        T2 second;

        Pair(T t, T2 t2) {
            first = t;
            second = t2;
        }
    };

    class TimeTrace {
    public:
        /**timestamp start*/
        unsigned long long timestamp;
        /**time cost records*/
        std::vector<Pair<ActionId, unsigned long long >> timeCostVector;

        /**flush all records to file*/
        void flush2File();

        /**clear all trace info*/
        void reset();
    };

}
#endif //LOGDOG_TIMETRACE_H
