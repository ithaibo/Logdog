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

#define LOG_PRINT true

struct Header {
    /**魔法值*/
    char magic[1 + LEN_HEADER_MAGIC];
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

    Header() {
        memset(magic, '\0', 1 + LEN_HEADER_MAGIC);
    }

    ~Header() {
        LOGI("[LogHeader] destroy");
    }
};

/**
 * 日志内容（可能是压缩、加密后的）
 */
struct Body {
    uint8_t *content = nullptr;
    ~Body() {
        LOGI("[LogBody] destroy");
    }
};

/**
 * 好买日志数据结构
 */
struct HbLog {
    Header *header = nullptr;
    Body *body = nullptr;
    uint32_t logLength;

    ~HbLog(){
        LOGI("[HbLog] destroy");
    }
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
//        LOGD("[protocol] log print, body.content:%s", log->body->content);
    } else if (!log->body) {
        LOGD("[protocol] log print, body nullptr");
    } else {
        LOGD("[protocol] log print, body->content nullptr");
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
    static std::shared_ptr<Header> createLogHeader(
            uint32_t type,
            unsigned long crc32,
            std::string *other,
            uint32_t bodyLen);

    /**
     * 创建日志对象
     * @param logContent 日志内容
     * @param lengthBody 内容长度
     */
    static std::shared_ptr<HbLog> createLogItem(uint8_t *logContent, size_t lengthBody);

    /**
     * 数据序列化
     * @param log
     * @return
     */
    static uint8_t *serialize(const HbLog *log);

    /**
     * 数据反序列化
     * @param toParse
     * @return
     */
    static std::shared_ptr<HbLog> deserialize(const uint8_t *toParse);

    static std::vector<std::shared_ptr<HbLog>> parseAll(const uint8_t *toParse, size_t length);
};

#endif //LOG_PROTOCOL_H
