//
// Created by haibo.wu on 2020/1/13.
//
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fstream>
#include <sys/stat.h>
#include <malloc.h>
#include "FileOption.h"
#include "alog.h"
#include "base64.h"


const char *FileOption::readFile(char const *filePath) {
    const size_t lengthF = obtainFileSize(filePath);
    LOGD("[mmap]: file size: %ld", lengthF);
    if (0 >= lengthF) {
        return nullptr;
    }
    int fd = open(filePath, O_RDONLY);
    if (fd == -1) {
        LOGE("[mmap]: file, path: %s open failed, reason: %s",
             filePath,
             strerror(errno));
        return nullptr;
    }

    char *bufferRead = (char *)mmap(NULL,
            lengthF,
            PROT_READ,
            MAP_SHARED,
            fd,
            0);
    if(bufferRead == MAP_FAILED) {
        return nullptr;
    }
    close(fd);
    if(nullptr != temp) {
        freeTempBuffer();
    }
    temp = (char *)malloc(lengthF+1);
    for (int i = 0; i < lengthF; ++i) {
        temp[i] = bufferRead[i];
    }
    temp[lengthF] = 0;
    munmap(bufferRead, lengthF);
    return temp;
}


size_t FileOption::getFileSize(const char *filePath) {
    if (nullptr == filePath) {
        return 0;
    }
    FILE *file = fopen(filePath, "re");
    if (nullptr == file) {
        return 0;
    }
    fseek(file, SEEK_SET, SEEK_END);
    long lengthF = ftell(file);
    fclose(file);
    return static_cast<size_t>(lengthF);
}


size_t FileOption::obtainFileSize(const char *path) {
    struct stat buff;
    if(stat(path, &buff) < 0) {
        return 0;
    }
    return static_cast<size_t>(buff.st_size);
}

void FileOption::freeTempBuffer() {
    if(nullptr != temp) {
        free(temp);
        temp = nullptr;
    }
}
