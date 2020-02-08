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
#include "logdog.h"


bool Buffer::mapMemory(const char *filePath, size_t size) {
    size_t fileSize = getFileSize(filePath);
    LOGI("[Buffer-mapMemory] file size: %ld", fileSize);

    //open file
    if (FD_NOT_OPEN == fd) {
        openFdForWriting(filePath);
    }
    if (FD_NOT_OPEN == fd) {
        return false;
    }

    off_t resultSeek;
    if (fileSize < 0) {
        //new file
        fileSize = 0;
    }

    resultSeek = ftruncate(fd, size + fileSize);
    lseek(fd, SEEK_SET, SEEK_END);
    if (-1 == resultSeek) {
        LOGE("[Buffer-mapMemory] set file size failed");
        close(fd);
        return false;
    } else {
        //seek success
        LOGD("[Buffer-mapMemory] set file size success, result lseek: %ld", resultSeek);
    }

    if (nullptr == bufferInternal) {
        //map memory
        LOGD("[Buffer-mapMemory] to map file...");
        bufferInternal = (char *) mmap(NULL,
                                       size + fileSize,
                                       PROT_WRITE | PROT_READ,
                                       MAP_SHARED,
                                       fd,
                                       0);
    }

    if (bufferInternal == MAP_FAILED) {
        LOGE("[Buffer-mapMemory]: mmap failed, reason: %s", strerror(errno));
        close(fd);
        fd = FD_NOT_OPEN;
        return false;
    }
    LOGD("[Buffer-mapMemory]: mmap success");

    return true;
}


bool Buffer::append(const char *path, const char *content) {
    size_t lengthStr = strlen(content);
    size_t lengthToSave = lengthStr * sizeof(char);

    if (!mapMemory(path, lengthToSave > bufferSize ? bufferSize : lengthToSave)) {
        LOGD("[Buffer-append] create map buffer failed");
        return false;
    }

    memcpy(bufferInternal, content, lengthStr);
    flush(lengthToSave);

//    size_t offLast = bufferSize - off;
//    LOGD("[Buffer-append] off last: %ld", offLast);
//    char *temp = (char *) content;
//    while (lengthToSave > 0) {
//        long last = (long) bufferSize - (long) off - (long) lengthToSave;
//        if (last > 0) {
//            LOGD("[Buffer-append] buffer last enough");
//            memcpy(bufferInternal, temp, lengthStr);
//            break;
//        } else if (last == 0) {
//            LOGD("[Buffer-append] buffer last equals length to save");
//            memcpy(bufferInternal + off, temp, lengthStr);
//            flush(bufferSize);
//            break;
//        } else {
//            LOGD("[Buffer-append] buffer last not enough, copy and map again");
//            memcpy(bufferInternal + off, temp, offLast);
//            LOGD("[Buffer-append] buffer last not enough, copy done");
//            lengthToSave -= offLast;
//            temp = temp + offLast;
//            offLast = 0;
//            flush(bufferSize);
//            LOGD("[Buffer-append] buffer last not enough, flush buffer done");
//            bool remap = mapMemory(path, lengthToSave);
//            if (!remap) {
//                LOGD("[Buffer-append] remap failed");
//                return false;
//            }
//            LOGD("[Buffer-append] buffer last not enough, remap buffer done");
//        }
//    }


    LOGD("[Buffer-append] success");
    return true;
}

void Buffer::flush(size_t size) {
    if (bufferInternal == nullptr) {
        return;
    }
    int unmapResult = munmap(bufferInternal, size);
    if (-1 == unmapResult) {
        LOGE("[unmap] error");
    }
    off = 0;
    bufferInternal = nullptr;
    LOGD("[Buffer-flush] done!");
}

char *Buffer::get(off_t start, size_t length) {
    if (off < start) return nullptr;
    if (length <= 0) return nullptr;
    char *copyStr = new char[length + 1];
    for (size_t index = 0; index < length; index++) {
        copyStr[index] = bufferInternal[index + start];
    }
    copyStr[length] = '\0';

    return copyStr;
}

bool Buffer::isBufferEnough(size_t size) {
    return ((bufferSize - off) >= size);
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

void Buffer::openFdForReading(const char *path) {
    if (path == nullptr) return;
    fd = open(path,
              O_RDONLY,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == FD_NOT_OPEN) {
        LOGE("[Buffer-OpenFd] open file for reading failed, reason: %s", strerror(errno));
        return;
    }
    LOGE("[Buffer-OpenFd] success");
}

void Buffer::setFilePath(const char *path) {
    filePath = path;
}
