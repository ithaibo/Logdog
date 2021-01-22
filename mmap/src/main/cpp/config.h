//
// Created by haibo.wu on 2021/1/22.
//

#ifndef LOGDOG_CONFIG_H
#define LOGDOG_CONFIG_H

constexpr uint32_t VERSION = 1;

constexpr const char *MAGIC = "HBLOG";

constexpr const char *ZLIB = "zlib";

constexpr uint32_t UINT32_LEN = sizeof(uint32_t);

/**写入文件的日志头-m魔法*/
constexpr uint32_t LEN_HEADER_MAGIC = 4;
constexpr uint32_t LEN_HEADER_HEADERLEN = UINT32_LEN;
constexpr uint32_t LEN_HEADER_TIMESTAMP = UINT32_LEN;
constexpr uint32_t LEN_HEADER_VERSION = 4;
constexpr uint32_t LEN_HEADER_ENCRYPT = 4;
constexpr uint32_t LEN_HEADER_ZIP = 4;
constexpr uint32_t LEN_HEADER_TYPE = UINT32_LEN;
constexpr uint32_t LEN_HEADER_CRC32 = 4;
constexpr uint32_t LEN_HEADER_OTHERLEN = UINT32_LEN;
constexpr uint32_t LEN_HEADER_BODYLEN = UINT32_LEN;

constexpr uint32_t calculateHeaderLength() {
    return
     LEN_HEADER_MAGIC
    +  LEN_HEADER_HEADERLEN
    +  LEN_HEADER_TIMESTAMP
    +  LEN_HEADER_VERSION
    +  LEN_HEADER_ENCRYPT
    +  LEN_HEADER_ZIP
    +  LEN_HEADER_TYPE
    +  LEN_HEADER_CRC32
    +  LEN_HEADER_OTHERLEN
    +  LEN_HEADER_BODYLEN ;
}

#endif //LOGDOG_CONFIG_H
