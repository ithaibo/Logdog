//
// Created by haibo.wu on 2020-01-27.
//

#ifndef LOGDOG_BUFFER_H

#define LOGDOG_BUFFER_H
#include <cstdio>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <memory>
#include "alog.h"

class Buffer {

public:
    static Buffer& get_instance(const char* path) {
        static Buffer buffer;
        if(nullptr == buffer.bufferInternal) {
            LOGD("[Buffer-getInstance] page size: %zu", buffer.BUFFER_UNIT_SIZE);
            buffer.setFilePath(path);
            buffer.initFile();
        }
        return buffer;
    }

    Buffer(size_t bufferSize);

    Buffer();

    bool isInit();
    inline void doInit(const char* path) {
        setFilePath(path);
        initFile();
    }

    void setFilePath(const char* path);
    const char* getFilePath();

    virtual /**
     * 追加写入
     * @param content 写入的内容
     * @param lengthToSave 需要保存的字符串长度
     * @return 写入的内容在缓冲中的起始位置
     */
    bool append(const uint8_t *content, size_t lengthToSave);

    void onExit();

protected:
    static const int FD_NOT_OPEN = -1;
    const size_t DEFAULT_BUFFER_SIZE = 64 * (size_t)getpagesize(); //256
    /**size of buffer*/
    size_t BUFFER_UNIT_SIZE = DEFAULT_BUFFER_SIZE;
    /**buffer map file*/
    uint8_t *bufferInternal = nullptr;
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

    bool init;

private:
    void initFile();

protected:
    /**
     * 创建一块新的映射区域
     * @param startOff 偏移位置
     * @return 是否成功
     */
    bool createNewBuffer(off_t startOff);
};

#endif //LOGDOG_BUFFER_H
