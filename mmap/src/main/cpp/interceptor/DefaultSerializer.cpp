//
// Created by Andy on 3/6/21.
//

#include "DefaultSerializer.h"
#include "../utils.h"

uint8_t *DefaultSerializer::visit(HbLog *log) {
    if (!log) {
        LOGE("[serialize] log pointer is null!");
        return nullptr;
    }
    if (log->header.bodyLen <= 0) {
        LOGE("[serialize] body length <= 0");
        return nullptr;
    }
    auto *temp = new uint8_t[log->logLength];
    uint32_t off = 0;
    memcpy(temp + off, log->header.magic, LEN_HEADER_MAGIC);
    off += LEN_HEADER_MAGIC;
    LOGW("[serialize]%s", "magic done");

    reverse((uint8_t*)&log->header.headerLen, LEN_HEADER_HEADERLEN);
    memcpy(temp + off, &log->header.headerLen, LEN_HEADER_HEADERLEN);
    off += LEN_HEADER_HEADERLEN;
    LOGW("[serialize]%s", "header len done");

    reverse((uint8_t*)&log->header.timestamp, LEN_HEADER_TIMESTAMP);
    memcpy(temp + off, &log->header.timestamp, LEN_HEADER_TIMESTAMP);
    off += LEN_HEADER_TIMESTAMP;
    LOGW("[serialize]%s", "timestamp done");

    reverse((uint8_t*)&log->header.version, LEN_HEADER_VERSION);
    memcpy(temp + off, &log->header.version, LEN_HEADER_VERSION);
    off += LEN_HEADER_VERSION;
    LOGW("[serialize]%s", "version done");

    reverse((uint8_t*)&log->header.encrypt, LEN_HEADER_ENCRYPT);
    memcpy(temp + off, &log->header.encrypt, LEN_HEADER_ENCRYPT);
    off += LEN_HEADER_ENCRYPT;
    LOGW("[serialize]%s", "encrypt done");

    reverse((uint8_t*)&log->header.zip, LEN_HEADER_ZIP);
    memcpy(temp + off, &log->header.zip, LEN_HEADER_ZIP);
    off += LEN_HEADER_ZIP;
    LOGW("[serialize]%s", "zip done");

    reverse((uint8_t*)&log->header.type, LEN_HEADER_TYPE);
    memcpy(temp + off, &log->header.type, LEN_HEADER_TYPE);
    off += LEN_HEADER_TYPE;
    LOGW("[serialize]%s", "type done");

    reverse((uint8_t*)&log->header.crc32, LEN_HEADER_CRC32);
    memcpy(temp + off, &log->header.crc32, LEN_HEADER_CRC32);
    off += LEN_HEADER_CRC32;
    LOGW("[serialize]%s", "crc32 done");

    const int lengthOther = log->header.otherLen;
    reverse((uint8_t*)&log->header.otherLen, LEN_HEADER_OTHERLEN);
    memcpy(temp + off, &log->header.otherLen, LEN_HEADER_OTHERLEN);
    off += LEN_HEADER_OTHERLEN;
    LOGW("[serialize]%s", "otherLen done");

    if (!log->header.other.empty() && log->header.otherLen > 0) {
        memcpy(temp + off, log->header.other.data(), lengthOther);
        off += log->header.otherLen;
        LOGW("[serialize]%s", "data done");
    }

    const int lengthBody = log->header.bodyLen;
    reverse((uint8_t*)&log->header.bodyLen, LEN_HEADER_BODYLEN);
    memcpy(temp + off, &log->header.bodyLen, LEN_HEADER_BODYLEN);
    off += LEN_HEADER_BODYLEN;
    LOGW("[serialize]%s", "bodyLen done");

    //body
    LOGW("[serialize] off:%d, remain:%d, length need:%d",
            off,
            log->logLength - off,
            log->header.bodyLen);
    memcpy(temp + off, log->body.content, lengthBody);
    LOGW("[serialize] body content done");
//    if (!log->body.content.empty()) {
//    } else {
//        LOGD("[debug] body or content is null");
//    }

    return temp;
}
