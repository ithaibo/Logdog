//
// Created by haibo.wu on 2021/1/22.
//

#ifndef LOG_PROTOCOL_H
#define LOG_PROTOCOL_H

#include <stdint.h>
#include <string>
#include "config.h"
#include "utils.h"

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
struct HbLog{
    LogHeader *header = nullptr;
    LogBody *body = nullptr;
};

std::shared_ptr<LogHeader> createLogHeader(
        uint32_t type,
        std::string crc32,
        std::string *other,
        uint32_t bodyLen);

#endif //LOG_PROTOCOL_H
