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




void MmapWrite(char const* filePath, char const* toSave) {
    if(nullptr == toSave) {
        LOGE("[mmap]: MmapWriteinvoked, toSave is null");
        return;
    }

    size_t pageSize = getFileSize(filePath);
    if(strlen(toSave) <= 0) {
        LOGE("[mmap]: MmapWriteinvoked, toSave is empty");
        return;
    }

    if(pageSize <= 0) {
        if(0>=strlen(toSave)) {
            LOGE("[mmap]: MmapWriteinvoked, file is empty, and toSave is empty, write empty to target file");
            writeFile(filePath, "");
        } else {
            writeFile(filePath, toSave);
        }
        return;
    }

    int fd = open(filePath, O_RDWR | O_CREAT | O_APPEND | O_CLOEXEC);
    if(fd == -1) {
        LOGE("[mmap]: file, path: %s open failed, reason: %s", filePath, strerror(errno));
        return;
    }

    size_t length = strlen(toSave);

    struct stat stat;
    fstat(fd, &stat);

//    lseek(fd, stat.st_size, SEEK_SET);

    //do mmap
    char* buffer = (char*)mmap(nullptr, stat.st_size, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
    size_t lengthFile = strlen(buffer);
    close(fd);

    if(nullptr == buffer) {
        LOGE("[mmap]: buffer pointer is null");
        return;
    }

    if(buffer == MAP_FAILED) {
        LOGE("[mmap]: mmap failed, reason: %s",strerror(errno));
        return;
    }

    LOGD("[mmap]: length of buffer: %lu", lengthFile);
    memcpy(buffer+lengthFile, toSave, length);
    munmap(buffer, stat.st_size);
}

const char* readWithMmap(char const *filePath) {
    size_t lengthF = getFileSize(filePath);
    LOGE("[mmap]: file size: %ld", lengthF);
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

    lseek(fd, 0, SEEK_SET);

    //get page size
    size_t pageSize = sizeof(filePath) * 20;//(size_t)getpagesize();
//    size_t pageSize = getFileSize(filePath);
    LOGD("[mmap]: page size: %lu", pageSize);

    struct stat stat;
    fstat(fd, &stat);
    //do mmap
    char* buffer = (char*)mmap(nullptr, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if(NULL == buffer) {
        LOGE("[mmap]: buffer pointer is null");
        return nullptr;
    }
    if(buffer == MAP_FAILED) {
        LOGE("[mmap]: mmap failed, reason: %s",strerror(errno));
        return nullptr;
    }
    LOGD("[mmap]: to get length of buffer map, %s", buffer);
    LOGI("[mmap]: read something from file, size: %d\n", pageSize);
    char* backContent = new char[pageSize];
//    memset(backContent, 0, (size_t)pageSize + 1);
    memcpy(backContent, buffer, (size_t)pageSize);
    munmap(buffer, stat.st_size);

    return backContent;
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
