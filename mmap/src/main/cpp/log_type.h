//
// Created by haibo.wu on 2021/1/22.
//

#ifndef LOGDOG_LOG_TYPE_H
#define LOGDOG_LOG_TYPE_H

enum LogType {
    /**端到端日志*/
    E2E = 1,

    /**崩溃日志*/
    CRASH,

    /**异常日志*/
    EXCEPTION,

    /**用户行为日志*/
    USER_OPERATION,

    /**性能日志*/
    PERFORMANCE,

    /**其他日志*/
    OTHER
};

#endif //LOGDOG_LOG_TYPE_H
