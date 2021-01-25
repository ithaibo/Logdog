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

#define LOG_PRINT false

struct LogHeader {
    /**魔法值*/
    char magic[4];
    /**header长度(文件)*/
    uint32_t headerLen;
    /**时间戳：写入时间*/
    u_long timestamp = getTimeStamp();
    /**存储软件版本号(整数字符串),对应config中的VERSION*/
    uint32_t version = VERSION;
    /**加密信息*/
    char encrypt[32];
    /**压缩方式 string,默认为zlib*/
    char zip[32];
    /**日志类型*/
    uint32_t type;
    /**body CRC32校验*/
    unsigned long crc32;
    /**保留区长度(文件)*/
    uint32_t otherLen;
    /**保留区内容*/
    uint8_t *other = nullptr;
    /**body长度(文件)*/
    uint32_t bodyLen;

    LogHeader() {}
    ~LogHeader() {
        LOGI("[LogHeader] destroy");
    }
};

/**
 * 日志内容（可能是压缩、加密后的）
 */
struct LogBody {
    uint8_t *content = nullptr;
};

/**
 * 好买日志数据结构
 */
struct HbLog {
    LogHeader *header = nullptr;
    LogBody *body = nullptr;
    uint32_t logLength;
};

std::shared_ptr<LogHeader> createLogHeader(
        uint32_t type,
        unsigned long crc32,
        std::string *other,
        uint32_t bodyLen);

inline void printLogHeader(const LogHeader *header) {
    if (!LOG_PRINT) return;

    if (!header) return;
    LOGD("[mmap] log print, header.magic:%s", header->magic);
    LOGD("[mmap] log print, header.headerLen:%d", header->headerLen);
    LOGD("[mmap] log print, header.timestamp:%ld", header->timestamp);
    LOGD("[mmap] log print, header.version:%d", header->version);
    LOGD("[mmap] log print, header.encrypt:%s", header->encrypt);
    LOGD("[mmap] log print, header.zip:%s", header->zip);
    LOGD("[mmap] log print, header.type:%d", header->type);
    LOGD("[mmap] log print, header.crc32:%ld", header->crc32);
    LOGD("[mmap] log print, header.otherLen:%d", header->otherLen);
    if (header->other)
        LOGD("[mmap] log print, header.other:%s", header->other);
    LOGD("[mmap] log print, header.bodyLen:%d", header->bodyLen);
}

inline void printLog(const HbLog *log) {
    if (!LOG_PRINT) return;
    if (!log) return;
    if (log->header) {
        LOGD("[mmap] log print, header.magic:%s", log->header->magic);
        LOGD("[mmap] log print, header.headerLen:%d", log->header->headerLen);
        LOGD("[mmap] log print, header.timestamp:%ld", log->header->timestamp);
        LOGD("[mmap] log print, header.version:%d", log->header->version);
        LOGD("[mmap] log print, header.encrypt:%s", log->header->encrypt);
        LOGD("[mmap] log print, header.zip:%s", log->header->zip);
        LOGD("[mmap] log print, header.type:%d", log->header->type);
        LOGD("[mmap] log print, header.crc32:%ld", log->header->crc32);
        LOGD("[mmap] log print, header.otherLen:%d", log->header->otherLen);
        if (log->header->other)
        LOGD("[mmap] log print, header.other:%s", log->header->other);
        LOGD("[mmap] log print, header.bodyLen:%d", log->header->bodyLen);
    }
    if (log->body && log->body->content) {
        LOGD("[mmap] log print, body.content:%s", log->body->content);
    }
}

#endif //LOG_PROTOCOL_H
