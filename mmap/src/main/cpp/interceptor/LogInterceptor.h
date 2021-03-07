//
// Created by Andy on 3/6/21.
//

#ifndef LOGDOG_LOGINTERCEPTOR_H
#define LOGDOG_LOGINTERCEPTOR_H


template<typename T, typename V>
class LogInterceptor {
public:
    virtual V *visit(T *input) = 0;
};

#endif //LOGDOG_LOGINTERCEPTOR_H
