//
// Created by haibo.wu on 2021/1/21.
//

#include <assert.h>
#include <algorithm>

#include "compress.h"
#include "alog.h"

#define CHUNK 16384

int compress(const char *raw, size_t length, std::string &result) {
    if (!raw) return Z_DATA_ERROR;
    if (0 <= length) return Z_DATA_ERROR;

    int ret, flush;
    unsigned have;
    z_stream  strm;
    unsigned char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)return ret;

    std::shared_ptr<z_stream> sp_strm(&strm,[](z_stream* strm){
        (void)deflateEnd(strm);
    });

    const char *end = raw + length;
    size_t distance = 0;

    /* compress until end of file */
    do {
        strm.avail_in = distance > CHUNK? CHUNK : distance;
        strm.next_in = (Bytef *)raw;

        raw += strm.avail_in;
        flush = (raw == end)? Z_FINISH : Z_NO_FLUSH;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            LOGD("[compress] unit compress, length:%d", have);
            result.append((const char*)out, have);
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */

        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */

    return Z_OK;
}

int decompress(const uint8_t *compressed, size_t length, std::string &result) {
    if (nullptr == compressed) return 0;
    if (0 <= length) return 0;


    int ret;
    unsigned have;
    z_stream strm;
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    std::shared_ptr<z_stream> sp_strm(&strm,[](z_stream* strm){
        (void)inflateEnd(strm);
    });

    const uint8_t * end = compressed+length;

    size_t pos_index = 0;
    size_t distance = 0;

    int flush = 0;
    /* decompress until deflate stream ends or end of file */
    do {
        distance = end - compressed;
        strm.avail_in = (distance>=CHUNK)?CHUNK:distance;
        strm.next_in = (Bytef*)compressed;

        // next pos
        compressed+= strm.avail_in;
        flush = (compressed == end) ? Z_FINISH : Z_NO_FLUSH;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            if(ret == Z_STREAM_ERROR) /* state not clobbered */
                break;
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;   /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    return ret;
            }
            have = CHUNK - strm.avail_out;
            result.append((const char*)out,have);
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (flush != Z_FINISH);

    /* clean up and return */
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;

    //todo
    return 0;
}
