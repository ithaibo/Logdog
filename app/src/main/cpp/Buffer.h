//
// Created by Andy on 2020-01-27.
//

#ifndef LOGDOG_BUFFER_H

#define LOGDOG_BUFFER_H
#include <cstdio>
#include <cstring>

class Buffer {
private:
    char *bufferInternal = nullptr;
    size_t bufferSize = 4 * 1024 * 1024 * 8;
    size_t off;

    void flush();

public:
    static Buffer& get_instance() {
        static Buffer buffer;
        return buffer;
    }

    bool mapMemory(const char* file_path);

    /**
     * 追加写入
     * @param content 写入的内容
     * @return 写入的内容在缓冲中的起始位置
     */
    off_t append(const char *content);

    /**
     * 从缓存中读取内容
     * @param start 开始位置
     * @param length 结束位置
     * @return
     */
    char *get(off_t start, size_t length);

    ~Buffer() {
        if(nullptr == bufferInternal)
            return;
        //unmap
        munmap(bufferInternal, bufferSize);
        //release buffer
        if(nullptr != bufferInternal)
            delete bufferInternal;
    }
};


#endif //LOGDOG_BUFFER_H
