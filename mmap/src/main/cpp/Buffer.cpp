//
// Created by haibo on 2020-01-27.
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>
#include <cstring>
#include <malloc.h>
#include <ctime>
#include <string>

#define LOG_TAG "MappedBuffer"

#include "Buffer.h"
#include "alog.h"
#include "FileOption.h"
#include "utils.h"
#include "MmapMain.h"

//typedef struct timeval TimeVal;

Buffer::Buffer(size_t bufferSize) {
    this->BUFFER_UNIT_SIZE = bufferSize;
}

Buffer::Buffer() {}

bool Buffer::append(const uint8_t *content, size_t lengthToSave) {
//    TimeVal timeStart;
//    gettimeofday(&timeStart, nullptr);

    //当前的buffer中还剩余的空间
    size_t lengthOff = BUFFER_UNIT_SIZE - actualSize;

//    LOGD("[Buffer-append] invoked, lengthToSave: %zu", lengthToSave);
//    LOGD("[Buffer-append] invoked, lengthOff: %zu", lengthOff);

    //check fd
    if(FD_NOT_OPEN == fd) {
        return false;
    }

    const uint8_t *temp = content;
    int copyTimes = 0;
    int bufferCreateTimes = 0;
    size_t saved;
    unsigned long long start, end;
    while (lengthToSave > 0) {
        if (lengthOff > 0) {
            start = getTimeUSDNow();
            saved = lengthToSave > lengthOff? lengthOff : lengthToSave;
//            LOGD("[Buffer-append] to copy, length:%d, content:%s", saved, temp);
            memcpy(bufferInternal + actualSize, temp, saved);
            copyTimes++;
            temp += saved;
            actualSize += saved;
            lengthToSave -= saved;
            lengthOff -= saved;
            end = getTimeUSDNow();
            Pair<ActionId, unsigned long long > save(ActionId::saveBuffer, end -start);
            MmapMain::trace->timeCostVector.push_back(save);
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
        bufferCreateTimes++;
        actualSize = 0;
        lengthOff = BUFFER_UNIT_SIZE;
//        LOGD("[Buffer-append] invoked, extend buffer done, legnthToSave:%zu, lengthOff;%zu", lengthToSave, lengthOff);
    }
    if (actualSize > 0) {
//        zeroFill(BUFFER_UNIT_SIZE - lengthOff, lengthOff);
        bufferInternal[actualSize] = '\0';
    }

//    LOGD("[Buffer-append] invoked, all saved done, copyTimes:%d, bufferCreateTimes:%d", copyTimes, bufferCreateTimes);

//    LOGD("[Buffer-append] success");
//    TimeVal timeEnd;
//    gettimeofday(&timeEnd, nullptr);
//    LOGI("[Buffer-append] invoked, time cost(suseconds):%ld", (timeEnd.tv_sec - timeStart.tv_sec) * 1000000 + (timeEnd.tv_usec - timeStart.tv_usec));
    return true;
}



void Buffer::setFilePath(const char *path) {
    char *temp = new char[strlen(path)];
    memcpy(temp, path, strlen(path));
    cachePath = temp;
}


bool Buffer::createNewBuffer(off_t startOff) {
//    TimeVal timeStart;
//    TimeVal timeEnd;
//    gettimeofday(&timeStart, nullptr);

    size_t sizeOld = fileSize;

    if(fd == FD_NOT_OPEN) {
        return false;
    }

    unsigned long long start, end;
    //unmap memory
    start = getTimeUSDNow();
    if(nullptr != bufferInternal && munmap(bufferInternal, BUFFER_UNIT_SIZE) != 0) {
        LOGE("[Buffer] release old buffer failed");
        return false;
    }
    end = getTimeUSDNow();
    Pair<ActionId, unsigned long long > unmap(ActionId::unmap, end -start);
    MmapMain::trace->timeCostVector.push_back(unmap);

    //set file size
    start = end;
    if(ftruncate(fd, static_cast<off_t>(fileSize + BUFFER_UNIT_SIZE)) != 0) {
        LOGE("[Buffer] extend file size failed");
        fileSize = sizeOld;
        return false;
    }
    end = getTimeUSDNow();
    Pair<ActionId, unsigned long long > ftruncate(ActionId::ftruncate, end -start);
    MmapMain::trace->timeCostVector.push_back(ftruncate);

    start = end;
    fileSize += BUFFER_UNIT_SIZE;
    bufferInternal = (uint8_t *)mmap(nullptr, BUFFER_UNIT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, startOff);
    if(bufferInternal == MAP_FAILED) {
        LOGE("[Buffer] create map memory failed, reason: %s", strerror(errno));
        return false;
    }
    end = getTimeUSDNow();
    Pair<ActionId, unsigned long long > mmap(ActionId::mmap, end -start);
    MmapMain::trace->timeCostVector.push_back(mmap);

//    gettimeofday(&timeEnd, nullptr);
//    LOGI("[Buffer-createNewBuffer] invoked, time cost(suseconds):%ld", (timeEnd.tv_sec - timeStart.tv_sec) * 1000000 + (timeEnd.tv_usec - timeStart.tv_usec));
    return true;
}



void Buffer::initFile() {
    if(init && nullptr != bufferInternal) {
        return;
    }

//    LOGD("[Buffer] initFile invoked");

    //if file not exists，create it
    //file exists，get size of it
    FileOption fileOption;
    this->fd = fileOption.openFdForMMAP(this->cachePath);
    if(this->fd == -1) {
        return;
    }
    size_t fileSizeNow = fileOption.obtainFileSize(this->cachePath);

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
    for (int i = BUFFER_UNIT_SIZE -1 ; i >= 0; i--) {
        if (bufferInternal[i] != '\0') {
            actualSize = i+1;
//            LOGD("[Buffer-init] correct actualSize: %zu, index:%d", actualSize, i);
            break;
        }

    }

    LOGD("[Buffer-initFile] file size: %zu, actualSize: %zu", fileSize, actualSize);
//    if (actualSize < BUFFER_UNIT_SIZE) {
//        bufferInternal[actualSize] = '\0';
//    }
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


const char *Buffer::getFilePath() {
    return cachePath;
}
