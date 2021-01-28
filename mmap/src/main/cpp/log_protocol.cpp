//
// Created by haibo.wu on 2021/1/22.
//

#include "log_protocol.h"
#include "utils.h"
#include <utility>
#include "compress/Zip.h"
#include "log_type.h"

Header LogProtocol::createLogHeader(uint32_t type, unsigned long crc32, std::string &other, uint32_t bodyLen) {
    Header header;
    memcpy(header.magic, MAGIC, LEN_HEADER_MAGIC);
    header.type = type;
    header.crc32 = crc32;
    header.bodyLen = bodyLen;
    header.encrypt = 0; //TODO 补全参数
    header.zip = 1; //TODO 补全参数
    header.otherLen = 0;
    if (!other.empty()) {
        header.otherLen = other.length();
    }
    if (header.otherLen > 0) {
        header.other = std::move(other);
    }
    header.headerLen = calculateHeaderLength();
    return header;
}

HbLog LogProtocol::createLogItem(std::string &logContent, size_t lengthBody) {
    HbLog log;
    log.body.content = std::move(logContent);
    LOGD("[bridge] sizeof(unsigned long):%d", sizeof(unsigned long ));
    unsigned long crc = crc32(0L, (Bytef*)log.body.content.data(), lengthBody);
    std::string emptyOther;
    log.header = createLogHeader(LogType::E2E, crc, emptyOther, lengthBody);
    log.logLength = calculateHeaderLength() + log.header.otherLen + log.header.bodyLen;
    return log;
}

/**
 * 将HbLog序列化为字节数组
 * @param log log
 * @return 字节数组
 */
uint8_t* LogProtocol::serialize(const HbLog &log) {
    if (log.body.content.empty()) return nullptr;
    auto *temp = new uint8_t[log.logLength];
    uint32_t off = 0;
    memcpy(temp + off, log.header.magic, LEN_HEADER_MAGIC);
    off += LEN_HEADER_MAGIC;
    memcpy(temp + off, &log.header.headerLen, LEN_HEADER_HEADERLEN);
    off += LEN_HEADER_HEADERLEN;
    memcpy(temp + off, &log.header.timestamp, LEN_HEADER_TIMESTAMP);
    off += LEN_HEADER_TIMESTAMP;
    memcpy(temp + off, &log.header.version, LEN_HEADER_VERSION);
    off += LEN_HEADER_VERSION;
    memcpy(temp + off, &log.header.encrypt, LEN_HEADER_ENCRYPT);
    off += LEN_HEADER_ENCRYPT;
    memcpy(temp + off, &log.header.zip, LEN_HEADER_ZIP);
    off += LEN_HEADER_ZIP;
    memcpy(temp + off, &log.header.type, LEN_HEADER_TYPE);
    off += LEN_HEADER_TYPE;
    memcpy(temp + off, &log.header.crc32, LEN_HEADER_CRC32);
    off += LEN_HEADER_CRC32;
    memcpy(temp + off, &log.header.otherLen, LEN_HEADER_OTHERLEN);
    off += LEN_HEADER_OTHERLEN;
    if (!log.header.other.empty() && log.header.otherLen > 0) {
        memcpy(temp + off, log.header.other.data(), log.header.otherLen);
        off += log.header.otherLen;
    }
    memcpy(temp + off, &log.header.bodyLen, LEN_HEADER_BODYLEN);
    off += LEN_HEADER_BODYLEN;

    //body
    if (!log.body.content.empty()) {
        memcpy(temp + off, log.body.content.data(), log.header.bodyLen);
    } else {
        LOGD("[debug] body or content is null");
    }

    return temp;
}

HbLog LogProtocol::parseOneLog(std::string &rawStr, LogRegionNeedParse positionAndLength) {
    auto *toParse = (const uint8_t*)rawStr.data();
    HbLog parsedLog;
    //parse header
    Header * header = &parsedLog.header;
    // deserialize
    uint32_t off = positionAndLength.magicPos;
    memcpy(header->magic, toParse + off, LEN_HEADER_MAGIC);
    off += LEN_HEADER_MAGIC;
    memcpy(&header->headerLen, toParse + off, LEN_HEADER_HEADERLEN);
    off += LEN_HEADER_HEADERLEN;
    if (header->headerLen >= positionAndLength.length) {//broken log, no need to parse
        LOGW("[protocol] parseOneLog failed, headerLen:%d >= region length:%d", header->headerLen, positionAndLength.length);
        return parsedLog;
    }
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
        header->other.append((const char *)toParse + off, header->otherLen);
        off += header->otherLen;
    }
    memcpy(&header->bodyLen, toParse + off, LEN_HEADER_BODYLEN);

    parsedLog.logLength = header->headerLen + header->bodyLen;
    if (header->bodyLen >= positionAndLength.length) {//broken log, no need to parse body
        LOGW("[protocol] parseOneLog failed, bodyLen >= region length");
        return parsedLog;
    }

    off += LEN_HEADER_BODYLEN;

    if (header->bodyLen > 0) {
        uLong crcCalculate = crc32(0L, toParse + off, header->bodyLen);
        bool crcMatch = crcCalculate == header->crc32;
        LOGD("crc32 match:%d", crcMatch);
        if (!crcMatch) {
            return parsedLog;
        }
        int codeDecompress = decompress(toParse + off, header->bodyLen, parsedLog.body.content);
        if (Z_OK != codeDecompress) {
            LOGE("[mmap] decompress log body failed");
        }
        parsedLog.header.bodyLen = parsedLog.body.content.length();
    }
    return parsedLog;
}

std::vector<LogRegionNeedParse> LogProtocol::parseAllMagicPosition(std::string &str) {
    size_t off = 0;
    std::vector<LogRegionNeedParse> magicOffPositions;
    auto * temp = (uint8_t*)str.data();
    char magicBuffer[5];
    magicBuffer[4] = '\0';
    //parse every log's magic position
    while (temp != nullptr && off < str.length()) {
        str.copy(magicBuffer, LEN_HEADER_MAGIC, off);
        if (0 == strcmp(magicBuffer, MAGIC)) {
            magicOffPositions.push_back(LogRegionNeedParse({off, 0}));
            LOGD("[protocol] magic position found:%d", off);
            off += LEN_HEADER_MAGIC;
            continue;
        }
        off++;
    }
    if (magicOffPositions.empty()) return magicOffPositions;
    //parse every log's length
    if (1 < magicOffPositions.size()) {
        for (int i = 1; i < magicOffPositions.size(); i++) {
            magicOffPositions.at(i - 1).length = magicOffPositions.at(i).magicPos - magicOffPositions.at(i - 1).magicPos;
            LOGD("[protocol] log index:%d, region length:%d", i-1, magicOffPositions.at(i-1).length);
        }
    }
    magicOffPositions.at(magicOffPositions.size()-1).length = str.length() - magicOffPositions.at(magicOffPositions.size()-1).magicPos;
    LOGD("[protocol] log index:%d, region length:%d", magicOffPositions.size()-1, magicOffPositions.at(magicOffPositions.size()-1).length);
    return magicOffPositions;
}