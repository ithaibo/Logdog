//
// Created by Andy on 2020-01-27.
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>
#include <cstring>
#include <malloc.h>
#include <ctime>

#define LOG_TAG "MappedBuffer"

#include "Buffer.h"
#include "alog.h"
#include "FileOption.h"

//typedef struct timeval TimeVal;

Buffer::Buffer(size_t bufferSize) {
    this->BUFFER_UNIT_SIZE = bufferSize;
}

Buffer::Buffer() {}

bool Buffer::append(const char *content) {
//    TimeVal timeStart;
//    gettimeofday(&timeStart, nullptr);

    size_t lengthStr = strlen(content);
    size_t lengthToSave = lengthStr * sizeof(char);
    //当前的buffer中还剩余的空间
    size_t lengthOff = BUFFER_UNIT_SIZE - actualSize;

//    LOGD("[Buffer-append] invoked, lengthToSave: %zu", lengthToSave);
//    LOGD("[Buffer-append] invoked, lengthOff: %zu", lengthOff);

    //check fd
    if(FD_NOT_OPEN == fd) {
        return false;
    }

    char *temp = const_cast<char *>(content);
    uint8_t *buffer = bufferInternal + actualSize;
    int copyTimes = 0;
    int bufferCreateTimes = 0;
    size_t saved;
    while (lengthToSave > 0) {
        if (lengthOff > 0) {
            saved = lengthToSave > lengthOff? lengthOff : lengthToSave;
//            LOGD("[Buffer-append] to copy, length:%d, content:%s", saved, temp);
            memcpy(bufferInternal + actualSize, temp, saved);
            copyTimes++;
            temp = temp + saved;
            actualSize += saved;
            lengthToSave -= saved;
            lengthOff -= saved;
//            LOGD("[Buffer-append] after copy, lengthToSave:%zu, lengthOff:%zu", lengthToSave, lengthOff);
            continue;
        }

        if (lengthToSave <= 0) {
            break;
        }
        //create new mapped buffer
//        LOGD("[Buffer-append] invoked, extend buffer...");
        if (!createNewBuffer(0)) {
            LOGE("[Buffer-append] invoked, extend buffer failed");
            return false;
        }
        buffer = bufferInternal;
        bufferCreateTimes++;
        actualSize = 0;
        lengthOff = BUFFER_UNIT_SIZE;
//        LOGD("[Buffer-append] invoked, extend buffer done, legnthToSave:%zu, lengthOff;%zu", lengthToSave, lengthOff);
    }
    if (lengthOff > 0) {
//        zeroFill(BUFFER_UNIT_SIZE - lengthOff, lengthOff);
        bufferInternal[BUFFER_UNIT_SIZE - lengthOff] = '\0';
    }

//    LOGD("[Buffer-append] invoked, all saved done, copyTimes:%d, bufferCreateTimes:%d", copyTimes, bufferCreateTimes);

//    LOGD("[Buffer-append] success");
//    TimeVal timeEnd;
//    gettimeofday(&timeEnd, nullptr);
//    LOGI("[Buffer-append] invoked, time cost(suseconds):%ld", (timeEnd.tv_sec - timeStart.tv_sec) * 1000000 + (timeEnd.tv_usec - timeStart.tv_usec));
    return true;
}



//note free memory
char *Buffer::get(off_t start, size_t length) {
    if (off < start) return nullptr;
    if (length <= 0) return nullptr;
    auto *copyStr = static_cast<uint8_t *>(malloc((length + 1) * sizeof(uint8_t)));
    memcpy(copyStr, bufferInternal, length);
    copyStr[length] = '\0';
    return reinterpret_cast<char *>(copyStr);
}



void Buffer::setFilePath(const char *path) {
    filePath = path;
}


bool Buffer::createNewBuffer(off_t startOff) {
//    TimeVal timeStart;
//    TimeVal timeEnd;
//    gettimeofday(&timeStart, nullptr);

    size_t sizeOld = fileSize;

    if(fd == FD_NOT_OPEN) {
        return false;
    }

    //unmap memory
    if(nullptr != bufferInternal && munmap(bufferInternal, BUFFER_UNIT_SIZE) != 0) {
        LOGE("[Buffer] release old buffer failed");
        return false;
    }

    //set file size
    if(ftruncate(fd, static_cast<off_t>(fileSize + BUFFER_UNIT_SIZE)) != 0) {
        LOGE("[Buffer] extend file size failed");
        fileSize = sizeOld;
        return false;
    }
    fileSize += BUFFER_UNIT_SIZE;
    bufferInternal = (uint8_t *)mmap(nullptr, BUFFER_UNIT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, startOff);
    if(bufferInternal == MAP_FAILED) {
        LOGE("[Buffer] create map memory failed, reason: %s", strerror(errno));
        return false;
    }

//    gettimeofday(&timeEnd, nullptr);
//    LOGI("[Buffer-createNewBuffer] invoked, time cost(suseconds):%ld", (timeEnd.tv_sec - timeStart.tv_sec) * 1000000 + (timeEnd.tv_usec - timeStart.tv_usec));
    return true;
}



void Buffer::initFile() {
    if (init) return;
    if(nullptr != bufferInternal) {
        return;
    }

//    LOGD("[Buffer] initFile invoked");

    //if file not exists，create it
    //file exists，get size of it
    FileOption fileOption;
    this->fd = fileOption.openFdForMMAP(this->filePath);
    if(this->fd == -1) {
        return;
    }
    size_t fileSizeNow = fileOption.obtainFileSize(this->filePath);

    //file size <0, obtain fd for writing and return.
    if (fileSizeNow <= 0) {
        if(ftruncate(fd, static_cast<off_t>(BUFFER_UNIT_SIZE)) != 0) {
            return;
        }
        fileSize = BUFFER_UNIT_SIZE;
        actualSize = 0;
    } else {
        fileSize = fileSizeNow;
    }

//    LOGD("[Buffer-initFile] file size: %zu, before create map memory", fileSize);

    //do memory map
    off_t startOff;
    if (fileSize <= BUFFER_UNIT_SIZE) {
        startOff = 0;
    } else {
        startOff = fileSize - BUFFER_UNIT_SIZE;
    }
    bufferInternal = (uint8_t *) mmap(nullptr, BUFFER_UNIT_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, this->fd, startOff);
    if (bufferInternal == MAP_FAILED) {
        bufferInternal = nullptr;
        init = false;
        LOGE("[Buffer-init] map file failed, reason: %s", strerror(errno));
        return;
    }
    //correct actual size
    for (int i = startOff; i < fileSize; i++) {
        if (bufferInternal[i] == '\0') {
            actualSize = i;
            if (startOff >= BUFFER_UNIT_SIZE) {
                 actualSize = actualSize % BUFFER_UNIT_SIZE;
            }
//            LOGD("[Buffer-init] correct actualSize: %zu, index:%d", actualSize, i);
            break;
        }
    }

//    LOGD("[Buffer-initFile] file size: %zu, actualSize: %zu", fileSize, actualSize);
    char temp[actualSize+1];
    temp[actualSize] = (char)0;
    memcpy(temp, bufferInternal, actualSize);
//    LOGD("[Buffer-initFile] content read from map: %s", temp);
    //release memory mapped
    init = true;
    LOGD("[Buffer-initFile] done");
}



void Buffer::onExit() {
    if(fd == FD_NOT_OPEN) return;
    if(nullptr == bufferInternal || MAP_FAILED == bufferInternal) {
        close(fd);
        return;
    }
    if(0 != msync(bufferInternal, fileSize, MS_SYNC)) {
        LOGE("sync failed");
        close(fd);
        return;
    }
    if(-1 == munmap(bufferInternal, fileSize)) {
        LOGE("unmap failed");
        close(fd);
        return;
    }

    bufferInternal = nullptr;
    close(fd);
}



bool Buffer::isInit() {
    return init;
}



char *Buffer::getAll() {
    return get(0, actualSize);
}
