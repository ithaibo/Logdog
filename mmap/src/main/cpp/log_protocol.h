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
#include <vector>

#define LOG_PRINT false

struct Header {
    /**魔法值*/
    char magic[1 + LEN_HEADER_MAGIC];
    /**header长度(文件)*/
    uint32_t headerLen;
    /**时间戳：写入时间*/
    uint64_t timestamp = getTimeStamp();
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
    std::string other;
    /**body长度(文件)*/
    uint32_t bodyLen;

    Header() {
        memset(magic, '\0', 1 + LEN_HEADER_MAGIC);
    }
};

/**
 * 日志内容（可能是压缩、加密后的）
 */
struct Body {
    std::string content;
};

/**
 * 好买日志数据结构
 */
struct HbLog {
    Header header;
    Body body;
    uint32_t logLength;
};


/**
 * 一条待解析的序列化日志数据的其实位置（MGAIC位置），及该条日志的长度
 */
struct LogRegionNeedParse {
    /**MGAIC位置*/
    size_t magicPos;
    /**该条日志的长度*/
    size_t length;
};


inline void printLogHeader(const Header *header) {
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
        if (!header->other.empty()) {
            LOGD("[protocol] log print, header.other:%s", header->other.c_str());
        }
        LOGD("[protocol] log print, header.bodyLen:%d", header->bodyLen);
    } catch (...) {
        LOGE("[protocol] exception occur during log print");
    }

}

inline void printLog(const HbLog &log) {
    if (!LOG_PRINT) return;
    if (log.logLength <= 0) return;
    printLogHeader(&log.header);
    if (!log.body.content.empty()) {
//        LOGD("[protocol] log print, body.content:%s", log->body->content);
    }
}

class LogProtocol {
public:
    /**
     * 创建LogHeader对象
     * @param type type
     * @param crc32 校验码
     * @param other 其他数据
     * @param bodyLen body长度
     * @return LogHeader的智能指针
     */
    static Header createLogHeader(uint32_t type, unsigned long crc32, std::string &other, uint32_t bodyLen);

    /**
     * 创建日志对象
     * @param logContent 日志内容
     * @param lengthBody 内容长度
     */
    static HbLog createLogItem(std::string &logContent, size_t lengthBody);

    /**
     * 数据序列化
     * @param log
     * @return
     */
    static uint8_t *serialize(const HbLog &log);

    static std::vector<LogRegionNeedParse> parseAllMagicPosition(std::string &str);

    static HbLog parseOneLog(std::string &rawStr, LogRegionNeedParse positionAndLength);
};

#endif //LOG_PROTOCOL_H
