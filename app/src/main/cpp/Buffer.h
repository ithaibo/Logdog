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
public:
    static Buffer& get_instance(const char* path) {
        static Buffer buffer;
        if(nullptr == buffer.bufferInternal) {
            buffer.initFile(path);
        }
        return buffer;
    }

    void setFilePath(const char* path);
    bool mapMemory(const char *path, size_t size);

    virtual /**
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

protected:
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

    /**当前文件的大小*/
    size_t fileSize = 0;

    /**当前文件写入内容的大小*/
    size_t actualSize = 0;

    void openFdForWriting(const char* path);
    void flush(size_t size);

private:
    void initFile(const char* path);
    bool isBufferEnough(size_t size);
    void openFdForReading(const char* path);

protected:
    /**
     * 确保文件大小
     * @param sizeNeed
     * @return 文件大小扩展是否成功
     */
    bool ensureFileSize(size_t sizeNeed);

    /**
     * 向新增加的文件中填充0
     * @param start 起始位置
     * @param length 填充的区间大小
     * @return 是否填充成功
     */
    bool zeroFill(off_t start, size_t length);
};


class WriteOnce : Buffer {
public:
    static WriteOnce& get_instance(const char* path) {
        static WriteOnce instance;
        return instance;
    }
    bool append(const char *path, const char *content) override;
};

#endif //LOGDOG_BUFFER_H
