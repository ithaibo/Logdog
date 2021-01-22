//
// Created by haibo.wu on 2021/1/21.
//

#ifndef LOGDOG_COMPRESS_H
#define LOGDOG_COMPRESS_H


#include <stdint.h>
#include <zlib.h>
#include <string>

/**
 * 压缩数据
 * @param raw 带压缩的输入数据
 * @param length 输入数据长度
 * @param result 压缩后的数据
 * @return 压缩后的数据长度
 */
int compress(const char *raw, size_t length, std::string &result);


/**
 * 解压缩数据
 * @param compressed 待解压缩的输入数据
 * @param length 输入数据长度
 * @param result 解压缩后的数据
 * @return 解压缩的数据长度
 */
int decompress(const uint8_t *compressed, size_t length, std::string &result);

#endif //LOGDOG_COMPRESS_H
