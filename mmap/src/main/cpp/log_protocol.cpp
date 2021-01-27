//
// Created by haibo.wu on 2021/1/22.
//

#include "log_protocol.h"
#include "utils.h"
#include <memory>
#include "compress/Zip.h"
#include "log_type.h"

std::shared_ptr<Header> LogProtocol::createLogHeader(
        uint32_t type,
        unsigned long crc32,
        std::string *other,
        uint32_t bodyLen) {
    std::shared_ptr<Header> header(new Header());

    memcpy(header->magic, MAGIC, LEN_HEADER_MAGIC);

    //type
    header->type = type;

    //crc32
//    assert(crc32.length() != 32);
    header->crc32 = crc32;

    //other
    header->other = nullptr;

    //body length
    header->bodyLen = bodyLen;

    //encrypt
    header->encrypt = 0; //TODO 补全参数
    //zip
    header->zip = 1; //TODO 补全参数
    header->otherLen = 0;
    if (other) {
        header->otherLen = other->length();
    }
    if (header->otherLen > 0) {
        header->other = new uint8_t [header->otherLen];
        memcpy(header->other, other->c_str(), header->otherLen);
    }

    //header length
    header->headerLen = calculateHeaderLength();

    return header;
}

std::shared_ptr<HbLog> LogProtocol::createLogItem(uint8_t *logContent, size_t lengthBody) {
    std::shared_ptr<HbLog> log(new HbLog());
    std::shared_ptr<Body> body(new Body());
    body->content = logContent;
    log->body = body.get();
    LOGD("[bridge] sizeof(unsigned long):%d", sizeof(unsigned long ));
    unsigned long crc = crc32(0L, body->content, lengthBody);
    log->header = createLogHeader(LogType::E2E, crc, nullptr, lengthBody).get();
    log->logLength = calculateHeaderLength() + log->header->otherLen + log->header->bodyLen;
    return log;
}

/**
 * 将HbLog序列化为字节数组
 * @param log log
 * @return 字节数组
 */
uint8_t* LogProtocol::serialize(const HbLog *log) {
    if (!log) return nullptr;
    if (!log->body || !log->body->content) return nullptr;
    assert(log->header != nullptr);
    assert(log->body != nullptr);

    auto *temp = new uint8_t[log->logLength];
    uint32_t off = 0;
    memcpy(temp + off, log->header->magic, LEN_HEADER_MAGIC);
    off += LEN_HEADER_MAGIC;
    memcpy(temp + off, &log->header->headerLen, LEN_HEADER_HEADERLEN);
    off += LEN_HEADER_HEADERLEN;
    memcpy(temp + off, &log->header->timestamp, LEN_HEADER_TIMESTAMP);
    off += LEN_HEADER_TIMESTAMP;
    memcpy(temp + off, &log->header->version, LEN_HEADER_VERSION);
    off += LEN_HEADER_VERSION;
    memcpy(temp + off, &log->header->encrypt, LEN_HEADER_ENCRYPT);
    off += LEN_HEADER_ENCRYPT;
    memcpy(temp + off, &log->header->zip, LEN_HEADER_ZIP);
    off += LEN_HEADER_ZIP;
    memcpy(temp + off, &log->header->type, LEN_HEADER_TYPE);
    off += LEN_HEADER_TYPE;
    memcpy(temp + off, &log->header->crc32, LEN_HEADER_CRC32);
    off += LEN_HEADER_CRC32;
    memcpy(temp + off, &log->header->otherLen, LEN_HEADER_OTHERLEN);
    off += LEN_HEADER_OTHERLEN;
    if (nullptr != log->header->other && log->header->otherLen > 0) {
        memcpy(temp + off, log->header->other, log->header->otherLen);
        off += log->header->otherLen;
    }
    memcpy(temp + off, &log->header->bodyLen, LEN_HEADER_BODYLEN);
    off += LEN_HEADER_BODYLEN;

    //body
    if (log->body && log->body->content) {
        memcpy(temp + off, log->body->content, log->header->bodyLen);
    } else {
        LOGD("[debug] body or content is null");
    }

    return temp;
}

std::shared_ptr<HbLog> LogProtocol::deserialize(const uint8_t *toParse) {
    std::shared_ptr<Header> header(new Header());
    // deserialize
    uint32_t off = 0;
    memcpy(header->magic, toParse + off, LEN_HEADER_MAGIC);
    if(0 != strcmp(header->magic, MAGIC)) {
        return nullptr;
    }
    off += LEN_HEADER_MAGIC;
    memcpy(&header->headerLen, toParse + off, LEN_HEADER_HEADERLEN);
    off += LEN_HEADER_HEADERLEN;
    memcpy(&header->timestamp, toParse + off, LEN_HEADER_TIMESTAMP);
    off += LEN_HEADER_TIMESTAMP;
    memcpy(&header->version, toParse + off, LEN_HEADER_VERSION);
    off += LEN_HEADER_VERSION;
    memcpy(&header->encrypt, toParse + off, LEN_HEADER_ENCRYPT);
    off += LEN_HEADER_ENCRYPT;
    memcpy(&header->zip, toParse + off, LEN_HEADER_ZIP);
    off += LEN_HEADER_ZIP;
    memcpy(&header->type, toParse + off, LEN_HEADER_TYPE);
    off += LEN_HEADER_TYPE;
    memcpy(&header->crc32, toParse + off, LEN_HEADER_CRC32);
    off += LEN_HEADER_CRC32;
    memcpy(&header->otherLen, toParse + off, LEN_HEADER_OTHERLEN);
    off += LEN_HEADER_OTHERLEN;
    if (header->otherLen > 0) {
        header->other = (uint8_t *) malloc(header->otherLen);
        memcpy(header->other, toParse + off, header->otherLen);
        off += header->otherLen;
    }
    memcpy(&header->bodyLen, toParse + off, LEN_HEADER_BODYLEN);
    off += LEN_HEADER_BODYLEN;

    std::shared_ptr<HbLog> parsedLog(new HbLog());
    parsedLog->header = header.get();
    parsedLog->logLength = header->headerLen + header->bodyLen;

    if (header->bodyLen > 0) {
        std::string decompressLogBody;
//        LOGD("start to calculate crc32, crc32 read from file:%lu, off:%u, body.length:%u", parsedLog->header->crc32, off, header->bodyLen);
        uLong crcCalculate = crc32(0L, toParse + off, header->bodyLen);
        bool crcMatch = crcCalculate == header->crc32;
        LOGD("crc32 match:%d", crcMatch);
        if (!crcMatch) {
        }
        int codeDecompress = decompress(toParse + off, header->bodyLen, decompressLogBody);
        if (Z_OK == codeDecompress) {
            std::shared_ptr<Body> logBody(new Body());
            //body
            logBody->content = (uint8_t *) decompressLogBody.c_str();
            parsedLog->body = logBody.get();
            parsedLog->header->bodyLen = decompressLogBody.length(); //TODO 是否可以考虑新增字段保存
//            LOGD("[bridge] header->bodyLen:%u, length decompressed:%d, strlen:%d", header->bodyLen, decompressLogBody.length(), strlen(decompressLogBody.c_str()));
        } else {
            LOGE("[mmap] decompress log body failed");
        }
    }

    return parsedLog;
}

std::vector<std::shared_ptr<HbLog>> LogProtocol::parseAll(const uint8_t *toParse, size_t length) {
    return std::vector<std::shared_ptr<HbLog>>();
}