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

bool FileOption::fileExists(const char *filePath) {
    int fd = open(filePath, O_RDONLY, (mode_t)0600);
    if(fd > 0) {
        close(fd);
        return true;
    }
    return false;
}

int FileOption::openFdForMMAP(const char *path) {
    return open(path, O_RDWR | O_CREAT, (mode_t)0600);
}

std::string FileOption::readFileAll(const char *filePath) {
    std::string readResult;
    int fd = open(filePath, O_RDONLY);
    if(fd < 0) {
        return readResult;
    }
    size_t fileLength = obtainFileSize(filePath);
    if (fileLength <= 0) {
        return readResult;
    }
    uint8_t buffer[4096];
    size_t n;
    while ((n =read(fd, buffer, 4096)) > 0) {
        readResult.append((const char *)buffer, n);
    }
    return readResult;
}
