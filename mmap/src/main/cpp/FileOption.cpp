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
    LOGD("[FileOptions] read all file, result address:%d", (&readResult));
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
    size_t off = 0;
    while ((n =read(fd, buffer, 4096)) > 0) {
        //shrink
        if (buffer[0] == '\0' && (n > 1 && buffer[1] == '\0')) {
            break;
        }
        readResult.append((const char *)buffer, n);
        off+=n;
    }
    return readResult;
}

void FileOption::writeFile(std::string &path, void *data) {
    int fd = open(path.c_str(), O_CREAT|O_APPEND, (mode_t)0600);
    if(fd < 0) {
        return;
    }
    uint8_t buffer[4096];
    size_t n;
    size_t off;
    while ((n = write(fd, data, 4096)) > 0) {
        off += n;
    }
    close(fd);
}
