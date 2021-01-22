//
// Created by haibo.wu on 2021/1/22.
//

#include "log_protocol.h"
#include "utils.h"

inline size_t min(size_t a, size_t b) {
    return a < b ? b : a;
}

std::shared_ptr<LogHeader> createLogHeader(
        uint32_t type,
        unsigned long crc32,
        std::string *other,
        uint32_t bodyLen) {
    LogHeader *header = new LogHeader();

    memcpy(header->magic, MAGIC, strlen(MAGIC));

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
    memcpy(header->encrypt, &"1", 1);
    //zip
    memcpy(header->zip, ZLIB, min(strlen(ZLIB), sizeof(header->zip)));
    header->otherLen = 0;

    //header length
    header->headerLen = calculateHeaderLength();

    return std::shared_ptr<LogHeader>(header);
}