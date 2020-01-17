//
// Created by haibo.wu on 2020/1/13.
//
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fstream>
#include <sys/stat.h>
#include "logdog.h"
#include "alog.h"
#include "base64.h"

#define NUMINTS  (52)
#define FILESIZE (NUMINTS * sizeof(char))

static char* buffer = nullptr;
static size_t length = 0;
static size_t off = 0;

void doMemcpy(const char* toSave, size_t lengthToSave) {
    if(nullptr == buffer) return;
    LOGD("[write]: do memory copy...");
    memcpy(buffer+length, toSave, lengthToSave);
    length+=lengthToSave;
    off -= lengthToSave;
    LOGD("[write]: do memory copy done, off now: %ld", off);
}

void MmapWrite(char const* filePath, char const* toSave) {
    if(nullptr == toSave) {
        LOGE("[mmap]: MmapWriteinvoked, toSave is null");
        return;
    }

    const char* saveBase64 = base64_encode(toSave);

    size_t lengthToSave = strlen(saveBase64);
    LOGD("[write]: content after base64: %s", saveBase64);

    unsigned int indexToCopy = 0;

    long offLong = off;
    long needLong = lengthToSave;
    if((offLong - needLong) <= 0 || nullptr == buffer) {
        LOGI("[append]: copy last space, buffer before copy: %s", buffer);
        LOGI("[append]: length: %ld", length);
        char *temp = nullptr;
        if(indexToCopy >0) {
            char arr[FILESIZE];
            temp = arr;
            memcpy(temp, buffer, FILESIZE);
            LOGI("[append]: buffer full: %s", temp);
        }

        int fd = open(filePath, O_RDWR | O_CREAT | O_TRUNC, O_APPEND);
        if(fd == -1) {
            LOGE("[mmap]: file, path: %s open failed, reason: %s", filePath, strerror(errno));
            return;
        }

        if(nullptr != buffer) {
            msync(buffer, FILESIZE, MS_SYNC);
            ftruncate(fd, length + FILESIZE);
            buffer = (char*)mremap(buffer, length, length + FILESIZE,MAP_SHARED);
            if(buffer == MAP_FAILED) {
                LOGE("[mmap]: mmap failed, reason: %s",strerror(errno));
                return;
            }
            if(nullptr != temp) {
                memcpy(buffer, temp, length);
                off += FILESIZE;
            }
        } else {
            LOGI("[mmap]: create map buffer");
            ftruncate(fd, FILESIZE);
            buffer = (char*)mmap(nullptr, FILESIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
            if(buffer == MAP_FAILED) {
                LOGE("[mmap]: mmap failed, reason: %s",strerror(errno));
                return;
            }
            off = FILESIZE;
        }
        close(fd);

        LOGE("[mmap]: length of buffer map: %ld", strlen(buffer));
    }

    doMemcpy(saveBase64 + indexToCopy, lengthToSave - indexToCopy);
}

const char* readFile(char const *filePath) {
    const size_t lengthF = getFileSize(filePath);
    LOGD("[mmap]: file size: %ld", lengthF);
    if(0 >= lengthF) {
        return nullptr;
    }
    int fd = open(filePath, O_RDONLY);
    if(fd == -1) {
        LOGE("[mmap]: file, path: %s open failed, reason: %s",
             filePath,
             strerror(errno));
        return nullptr;
    }
    char* bufferRead = new char[lengthF+1];
    read(fd, bufferRead, lengthF);
    return base64_decode(bufferRead);
}

void writeFile(const char *filePath, const char *contentSave) {
    std::fstream fout;
    fout.open(filePath, std::ios::out | std::ios::app);
    if (!fout.is_open()) {
        LOGE("open file, %s failed", filePath);
    } else {
        fout.write(contentSave, strlen(contentSave));
        fout.close();
    }
}


size_t getFileSize(const char *filePath) {
    if(nullptr == filePath) {
        return 0;
    }
    FILE *file = fopen(filePath, "re");
    if(nullptr == file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long lengthF = ftell(file);
    fclose(file);
    return static_cast<size_t>(lengthF);
}
