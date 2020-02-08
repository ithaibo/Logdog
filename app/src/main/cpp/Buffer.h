//
// Created by Andy on 2020-01-27.
//

#ifndef LOGDOG_BUFFER_H

#define LOGDOG_BUFFER_H
#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

class Buffer {
private:
    static const int FD_NOT_OPEN = -1;

    /**buffer map file*/
    char *bufferInternal = nullptr;
    /**size of buffer*/
    size_t bufferSize = (size_t)getpagesize();
    /**off index of file*/
    size_t off;
    /**file descriptor: file not open*/
    int fd = FD_NOT_OPEN;
    /**file path to save log */
    const char* filePath;

    bool isBufferEnough(size_t size);
    void openFdForWriting(const char* path);
    void openFdForReading(const char* path);

public:
    static Buffer& get_instance(const char* path) {
        static Buffer buffer;
        return buffer;
    }

    void setFilePath(const char* path);

    bool mapMemory(const char *path, size_t size);

    /**
     * 追加写入
     * @param content 写入的内容
     * @return 写入的内容在缓冲中的起始位置
     */
    bool append(const char *path, const char *content);

    /**
     * 从缓存中读取内容
     * @param start 开始位置
     * @param length 结束位置
     * @return
     */
    char *get(off_t start, size_t length);
    void flush(size_t size);
};


#endif //LOGDOG_BUFFER_H
