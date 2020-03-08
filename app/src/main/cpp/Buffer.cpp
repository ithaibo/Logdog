//
// Created by Andy on 2020-01-27.
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <error.h>
#include <errno.h>
#include <cstring>
#include "Buffer.h"
#include "alog.h"
#include "FileOption.h"
#include "base64.h"


static const size_t UNIT_SIZE = 4096;


bool Buffer::append(const char *content) {
    size_t lengthStr = strlen(content);
    size_t lengthToSave = lengthStr * sizeof(char);

    LOGD("[Buffer-append] invoked, lengthToSave: %u", lengthToSave);

    //check fd
    if(FD_NOT_OPEN == fd) {
        return false;
    }

    if(!ensureFileSize(lengthToSave)) {
        return false;
    }

    LOGD("[Buffer-append] actualSize: %u", actualSize);
    LOGD("[Buffer-append] buffer str length: %u", strlen(bufferInternal));
    for (int i = 0; i <lengthStr; ++i) {
        bufferInternal[actualSize+i] = content[i];
    }
    actualSize += lengthToSave;

    LOGD("[Buffer-append] success");
    return true;
}

char *Buffer::get(off_t start, size_t length) {
    if (off < start) return nullptr;
    if (length <= 0) return nullptr;
    //fixme memory leak
    char *copyStr = new char[length + 1];
    for (size_t index = 0; index < length; index++) {
        copyStr[index] = bufferInternal[index + start];
    }
    copyStr[length] = '\0';

    return copyStr;
}

void Buffer::openFdForWriting(const char *path) {
    if (path == nullptr) return;
    fd = open(path,
              O_RDWR | O_CREAT | O_TRUNC,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == FD_NOT_OPEN) {
        LOGE("[Buffer-OpenFd] open file for writing failed, reason: %s", strerror(errno));
        return;
    }
    LOGD("[Buffer-OpenFd] success");
}

void Buffer::setFilePath(const char *path) {
    filePath = path;
}

bool Buffer::ensureFileSize(size_t sizeNeed) {
    size_t sizeOld = fileSize;

    //如果当前的文件和缓冲区大小满足写入的内容，直接返回
    if ((sizeOld - actualSize) > sizeNeed) {
        LOGI("[Buffer] size of file and buffer in memory is enough, no need increase");
        return true;
    }

    if(fd == FD_NOT_OPEN) {
        return false;
    }
    //增大文件大小，每次最小增加4K
    while (fileSize < (actualSize + sizeNeed)) {
        fileSize += UNIT_SIZE;
    }
    size_t sizeIncreased = (fileSize - sizeOld);
    LOGD("[Buffer] file extend, need: %u, size increased: %u", sizeNeed, sizeIncreased);
    if(sizeIncreased <= 0) {
        return true;
    }
    //set file size
    if(ftruncate(fd, static_cast<off_t>(fileSize)) != 0) {
        LOGE("[Buffer] extend file size failed");
        return false;
    }
    LOGD("[Buffer] extend file size succeed");

    //fill zero
    if(!zeroFill(static_cast<off_t>(sizeOld), fileSize - sizeOld)) {
        return false;
    }
    LOGD("[Buffer] fill zero succeed");

    //unmap memory
    if(nullptr != bufferInternal) {
        if(munmap(bufferInternal, sizeOld) != 0) {
            LOGE("[Buffer] release old buffer failed");
            return false;
        }
    }
    bufferInternal = (char*)mmap(bufferInternal,
            fileSize,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            fd,
            0);
    if(bufferInternal == MAP_FAILED) {
        LOGE("[Buffer] create map memory failed, reason: %s", strerror(errno));
        return false;
    }
    LOGD("[Buffer] create map memory succeed");

    return true;
}

bool Buffer::zeroFill(off_t start, size_t length) {
    if(start < 0 || length <= 0) {
        return false;
    }
    if(0 > lseek(fd, start, SEEK_SET)) {
        LOGE("[Buffer] lseek failed");
        return false;
    }
    static const char zeros[UNIT_SIZE] = {0};
    while (length >= sizeof(zeros)) {
        if (write(fd, zeros, sizeof(zeros)) < 0) {
            LOGE("fail to write fd[%d], error:%s", fd, strerror(errno));
            return false;
        }
        length -= sizeof(zeros);
    }
    if (length > 0) {
        if (write(fd, zeros, length) < 0) {
            LOGE("fail to write fd[%d], error:%s", fd, strerror(errno));
            return false;
        }
    }
    return true;
}

void Buffer::initFile() {
    if(nullptr != bufferInternal) {
        return;
    }

    LOGD("[Buffer] initFile invoked");
    if(access(this->filePath, F_OK) != -1) {
        //读取文件大小
        FileOption *fileOption = new FileOption();
        size_t fileSizeNow = fileOption->obtainFileSize(this->filePath);
        LOGD("[Buffer-initFile] file size: %u, before create map memory", fileSizeNow);
        if(fileSizeNow > 0) {
            //ensure
            size_t sizeMap = fileSizeNow;
            const char* bufferRead = nullptr;
            //读取文件到缓冲区
            bufferRead = fileOption->readFile(this->filePath);
            actualSize = fileSizeNow;
            for (unsigned int i = 0; i < fileSizeNow; ++i) {
                if (bufferRead[i] == 0) {
                    actualSize = i;
                    LOGD("[Buffer-initFile] ensure actual size is: %u", actualSize);
                    break;
                }
            }
            bool needExtendFileSize = false;
            if (actualSize != fileSizeNow) {
                needExtendFileSize = true;
            }
            //保证文件大小设置UNIT_SIZE的整数倍
            if ((sizeMap % UNIT_SIZE) != 0) {
                sizeMap = UNIT_SIZE * (1 + fileSizeNow / UNIT_SIZE);
                needExtendFileSize = true;
            }
            LOGD("[Buffer-initFile] target file size: %u", sizeMap);
            openFdForWriting(this->filePath);
            //扩展文件大小
            if(needExtendFileSize) {
                if (0 != ftruncate(fd, static_cast<off_t>(sizeMap))) {
                    LOGE("[Buffer-initFile] extend file size failed, reason: %s", strerror(errno));
                    _exit(1);
                }
            }
            //执行内存映射
            bufferInternal = (char *)mmap(NULL, sizeMap, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
            if(bufferInternal == MAP_FAILED) {
                bufferInternal = nullptr;
                LOGD("[Buffer-initFile] file exists, before create map memory", fileSizeNow);
                return;
            }
            if(nullptr != bufferInternal) {
                fileSize = sizeMap;
//                actualSize = fileSizeNow;
                LOGD("[Buffer-initFile] file size: %u, actualSize: %u", fileSize, actualSize);
            }
            LOGD("[Buffer-initFile] write back content...");
            //内存拷贝
            for (int i = 0; i <actualSize && nullptr != bufferRead; ++i) {
                bufferInternal[i] = bufferRead[i];
            }
            //释放读缓冲区
            fileOption->freeTempBuffer();
            LOGD("[Buffer-initFile] writing done");
        } else {
            fileSize = 0;
            openFdForWriting(this->filePath);
        }

    } else {
        fileSize = 0;
        openFdForWriting(this->filePath);
    }
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

    if(actualSize < fileSize) {
        //truncate fileSize
        LOGI("shrink file to: %u", actualSize);
        if(0 != ftruncate(fd, static_cast<off_t>(actualSize))) {
            LOGE("shrink file failed");
        } else {
            LOGI("shrink file success, now file size: %u", actualSize);
        }
    }
    bufferInternal = nullptr;
    close(fd);
}
