//
// Created by Andy on 2021/1/16.
//

#ifndef LOGDOG_ZIP_H
#define LOGDOG_ZIP_H


#include <zlib.h>
#include <string>


int compress(const char* inString, size_t inLength,
             std::string& out_str, int level);

int decompress(const char* str2Decompress, size_t length2Decompress,
               std::string& outStr);

#endif //LOGDOG_ZIP_H
