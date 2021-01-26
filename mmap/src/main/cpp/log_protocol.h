//
// Created by haibo.wu on 2021/1/22.
//

#ifndef LOG_PROTOCOL_H
#define LOG_PROTOCOL_H

#include <stdint.h>
#include <string>
#include "config.h"
#include "utils.h"
#include "alog.h"
#include <exception>

#define LOG_PRINT true

struct LogHeader {
    /**魔法值*/
    char magic[4];
    /**header长度(文件)*/
    uint32_t headerLen;
    /**时间戳：写入时间*/
    u_long timestamp = getTimeStamp();
    /**存储软件版本号(整数字符串),对应config中的VERSION*/
    uint32_t version = VERSION;
    /**加密 1:加密；0：未加密*/
    uint32_t encrypt;
    /**压缩(zlib) 1:压缩；0：未压缩*/
    uint32_t zip;
    /**日志类型. 对应log_type.h中的LogType*/
    uint32_t type;
    /**body CRC32校验*/
    unsigned long crc32;
    /**保留区长度(文件)*/
    uint32_t otherLen;
    /**保留区内容*/
    uint8_t *other = nullptr;
    /**body长度(文件)*/
    uint32_t bodyLen;

    ~LogHeader() {
        LOGI("[LogHeader] destroy");
    }
};

/**
 * 日志内容（可能是压缩、加密后的）
 */
struct LogBody {
    uint8_t *content = nullptr;
    ~LogBody() {
        LOGI("[LogBody] destroy");
    }
};

/**
 * 好买日志数据结构
 */
struct HbLog {
    LogHeader *header = nullptr;
    LogBody *body = nullptr;
    uint32_t logLength;

    ~HbLog(){
        LOGI("[HbLog] destroy");
    }
};

std::shared_ptr<LogHeader> createLogHeader(
        uint32_t type,
        unsigned long crc32,
        std::string *other,
        uint32_t bodyLen);

inline void printLogHeader(const LogHeader *header) {
    if (!LOG_PRINT) return;

    if (!header) return;
    try {
        LOGD("[protocol] log print, header.magic:%s", header->magic);
        LOGD("[protocol] log print, header.headerLen:%d", header->headerLen);
        LOGD("[protocol] log print, header.timestamp:%lu", header->timestamp);
        LOGD("[protocol] log print, header.version:%d", header->version);
        LOGD("[protocol] log print, header.encrypt:%d", header->encrypt);
        LOGD("[protocol] log print, header.zip:%d", header->zip);
        LOGD("[protocol] log print, header.type:%d", header->type);
        LOGD("[protocol] log print, header.crc32:%lu", header->crc32);
        LOGD("[protocol] log print, header.otherLen:%d", header->otherLen);
        if (header->other) {
            LOGD("[protocol] log print, header.other:%s", header->other);
        }
        LOGD("[protocol] log print, header.bodyLen:%d", header->bodyLen);
    } catch(...) {
        LOGE("[protocol] exception occur during log print");
    }

}

inline void printLog(const HbLog *log) {
    if (!LOG_PRINT) return;
    if (!log) return;
    printLogHeader(log->header);
    if (log->body && log->body->content) {
        LOGD("[protocol] log print, body.content:%s", log->body->content);
    } else if (!log->body) {
        LOGD("[protocol] log print, body nullptr");
    } else {
        LOGD("[protocol] log print, body->content nullptr");
    }
}

#endif //LOG_PROTOCOL_H
