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

Buffer::Buffer(size_t bufferSize) {
    this->BUFFER_UNIT_SIZE = bufferSize;
}

Buffer::Buffer() {}

void Buffer::initFile() {
    //init mmap buffer
    if(!createNewBuffer()) {
        return;
    }
    //parse data length
    size_t actualSize = parseDaraLength();
    //parse last update time
    this->timestampUpdate = parseTimestamp();

    if(actualSize > 0) {
        // flush to file
        flush();
    } else {
        this->off = actualSize + POSITION_DATA;
    }
    this->init = true;
    LOGD("[Buffer-initFile] done");
}

bool Buffer::createNewBuffer() {
    FileOption fileOption;
    bool cacheFileExisted = fileOption.fileExists(this->cachePath);
    this->fd = fileOption.openFdForMMAP(this->cachePath);
    if(this->fd == -1) {
        LOGE("[Buffer] cache file open failed!");
        return false;
    }
    if(ftruncate(fd, static_cast<off_t>(BUFFER_UNIT_SIZE)) != 0) {
        LOGE("[Buffer] truncate file failed!");
        return false;
    }
    this->bufferInternal = (uint8_t *) mmap(nullptr,
                                            BUFFER_UNIT_SIZE,
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED,
                                            this->fd,
                                            0);
    if(this->bufferInternal == MAP_FAILED) {
        LOGE("[Buffer] create map memory failed, reason: %s", strerror(errno));
        return false;
    }
    if(!cacheFileExisted) {
        updateDataLength(0);
        updateTimestamp(0);
    }
    return true;
}

bool Buffer::append(const uint8_t *content, size_t lengthToSave) {
    if (!content) {
        LOGE("[Buffer] content pointer is null");
        return false;
    }
    if(!this->bufferInternal) {
        LOGE("[Buffer] not init yet!");
        return false;
    }
    const uint8_t *temp = content;
    int copyTimes = 0;
    size_t saved;
    while (lengthToSave > 0) {
        if (remain() > 0) {
            LOGI("[Buffer] length to save:%d, off now:%d, remain:%d", lengthToSave, off, remain());
            saved = lengthToSave > remain()? remain() : lengthToSave;
            memcpy(this->bufferInternal + this->off, temp, saved);
            copyTimes++;
            this->off += saved;
            temp += saved;
            lengthToSave -= saved;
            updateDataLength(off - POSITION_DATA);
            this->timestampUpdate = getTimeStamp();
            updateTimestamp(this->timestampUpdate);
            continue;
        }
        if (lengthToSave <= 0) {
            break;
        }
        LOGI("[Buffer-append] buffer full, flush data to file.");
        flush();
    }
    LOGD("[Buffer] cache done, off now:%d, remain:%d", off, remain());
    return true;
}

void Buffer::setFilePath(const char *path) {
    //cache path
    std::string cachePath(path);
    cachePath.append("/");
    cachePath.append("cache");
    LOGI("[Buffer] set file path, cache path:%s", cachePath.c_str());
    char *temp = new char[cachePath.length()];
    memcpy(temp, cachePath.c_str(), cachePath.length());
    this->cachePath = temp;

    //log dir
    this->logDir.append(path);
    //log file path
    this->logPath = path;
    this->logPath.append("/");
    this->logPath.append(format_string("%ld.log", getTimeStamp()));
}

void Buffer::onExit() {
    if(this->fd == FD_NOT_OPEN) return;
    if(nullptr == bufferInternal || MAP_FAILED == this->bufferInternal) {
        close(this->fd);
        return;
    }
//    if(0 != msync(this->bufferInternal, BUFFER_UNIT_SIZE, MS_SYNC)) {
//        LOGE("sync failed");
//        close(this->fd);
//        return;
//    }
    if(-1 == munmap(this->bufferInternal, BUFFER_UNIT_SIZE)) {
        LOGE("unmap failed");
        close(this->fd);
        return;
    }

    this->bufferInternal = nullptr;
    close(this->fd);
}

bool Buffer::isInit() const {
    return this->init;
}

const char *Buffer::getFilePath() {
    return this->cachePath;
}

size_t Buffer::parseDaraLength() {
    if(!this->bufferInternal) return 0;
    uint32_t dataLength;
    memcpy(&dataLength, bufferInternal, POSITION_TIMESTAMP - POSITION_LENGTH);
    if(isLittleEndian()) {
        reverse((uint8_t *) &dataLength, POSITION_TIMESTAMP - POSITION_LENGTH);
    }
    return dataLength;
}

uint64_t Buffer::parseTimestamp() {
    uint8_t *tmp = bufferInternal + POSITION_TIMESTAMP;
    uint64_t timestamp;
    const size_t len = POSITION_DATA - POSITION_TIMESTAMP;
    memcpy(&timestamp, tmp, len);
    if(isLittleEndian()) {
        reverse((uint8_t*)&timestamp, len);
    }
    return timestamp;
}

void Buffer::flush() {
    const int lengthToSave = off - POSITION_DATA;
    if(lengthToSave <= 0) {
        LOGE("[Buffer] no content to flush");
        return;
    }
    LOGI("[Buffer] flush invoked, path:%s, data length:%d", logPath.c_str(), off - POSITION_DATA);
    FileOption::writeFile(this->logPath.c_str(), this->bufferInternal, off - POSITION_DATA);
    this->timestampUpdate = 0;
    this->off = POSITION_DATA;
}

size_t Buffer::remain() const {
    return BUFFER_UNIT_SIZE - off;
}

void Buffer::updateDataLength(size_t length) {
    LOGI("update date length:%d", length);
    uint8_t *tmp = this->bufferInternal + POSITION_LENGTH;
    if(isLittleEndian()) {
        reverse((uint8_t*)&length, POSITION_TIMESTAMP);
    }
    memcpy(tmp, &length, POSITION_TIMESTAMP);
}

void Buffer::updateTimestamp(uint64_t timestamp) {
    LOGI("update timestamp:%lld", timestamp);
    uint8_t *tmp = this->bufferInternal + POSITION_TIMESTAMP;
    if(isLittleEndian()) {
        reverse((uint8_t *) &timestamp, POSITION_DATA - POSITION_TIMESTAMP);
    }
    memcpy(tmp, &timestamp, POSITION_DATA - POSITION_TIMESTAMP);
}

Buffer::~Buffer() {
    delete [] this->cachePath;
}
