//
// Created by Andy on 2021/1/16.
//

#include "Zip.h"

#define CHUNK 16384
#define LOG_TAG "Zip"
#include "../alog.h"

int compress(const char *inString, size_t inLength,
             std::string &out_str, int level) {
    if (!inString) return Z_DATA_ERROR;

    int ret;
    int flush;
    unsigned have;
    unsigned char out[CHUNK];

    /// allocate deflate state
    z_stream stream;
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    ret = deflateInit(&stream, level);
    if (ret != Z_OK) {
        LOGE("[compress] deflateInit error");
        return ret;
    }

    const char* end = inString + inLength;

    size_t distance;

    /// compress until end of file
    do {
        distance = end - inString;
        stream.avail_in = (distance >= CHUNK)? CHUNK : distance;
        stream.next_in = (Bytef *)inString;
        /// next pos
        inString += stream.avail_in;
        flush = (inString == end)? Z_FINISH : Z_NO_FLUSH;
        /// run deflate() on input until output buffer not full, finish
        /// compression if all of source has been read in
        do {
            stream.avail_out = CHUNK;
            stream.next_out = out;
            ret = deflate(&stream, flush); /// no bad return value
            if (ret == Z_STREAM_ERROR) {
                break;
            }
            have = CHUNK - stream.avail_out;
            out_str.append((const char*)out, have);
        } while (stream.avail_out == 0);
        if (stream.avail_in != 0) break; /// all input will be used
    } while (flush != Z_FINISH); /// done when last data in file processed

    if (ret != Z_STREAM_END) return Z_STREAM_ERROR;

    /// clean up and return
    return Z_OK;
}

int decompress(const char *str2Decompress, size_t length2Decompress,
               std::string& outStr) {
    if (!str2Decompress) return Z_DATA_ERROR;

    int ret;
    unsigned have;
    z_stream stream;
    unsigned char out[CHUNK];

    /// allocate infalte state
    stream.zalloc = nullptr;
    stream.zfree = nullptr;
    stream.opaque = nullptr;
    stream.avail_in = 0;
    stream.next_in = nullptr;
    ret = inflateInit(&stream);
    if (ret != Z_OK) {
        LOGE("decompress inflateInit failed");
        return ret;
    }

    /// address of last char to decompress
    const char* end = str2Decompress + length2Decompress;

    size_t distance;

    int flush;

    /// decompress until deflate stream ends or end of file
    do{
        distance = end - str2Decompress;
        stream.avail_in = (distance > CHUNK)? CHUNK : distance;
        stream.next_in = (Bytef*)str2Decompress;
        /// next position
        str2Decompress += stream.avail_in;
        flush = (str2Decompress == end)? Z_FINISH : Z_NO_FLUSH;

        /// run inflate() on input until output buffer not full
        do {
            stream.avail_out = CHUNK;
            stream.next_out = out;
            ret = inflate(&stream, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) break;/// state not clobbered

            switch (ret) {
                case Z_NEED_DICT: ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR: return ret;
                default:;
            }

            have = CHUNK - stream.avail_out;
            outStr.append((const char*)out, have);
        }while (stream.avail_out == 0);
        /// done when inflate() says it's done
    } while (flush != Z_FINISH);

    return ret == Z_STREAM_END? Z_OK : Z_DATA_ERROR;
}
