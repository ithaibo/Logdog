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
#include "logdog.h"
#include "alog.h"
#include "base64.h"

#define NUMINTS  (52)
#define FILESIZE (NUMINTS * sizeof(char))
#define    FILE_MODE    (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

static size_t length = 0;
static size_t off = 0;

static char *logBuffer = nullptr;



const char *readFile(char const *filePath) {
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
    const int length = lengthF;
    char temp[length];
    memcpy(temp, bufferRead, lengthF);
    munmap(bufferRead, lengthF);
    return temp;
}

void writeFile(const char *filePath, const char *contentSave) {
    std::fstream fout;
    fout.open(filePath, std::ios::out/* | std::ios::app*/);
    if (!fout.is_open()) {
        LOGE("open file, %s failed", filePath);
    } else {
        fout.write(/*base64_encode(*/contentSave/*)*/, strlen(contentSave));
        fout.close();
    }
}


size_t getFileSize(const char *filePath) {
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

size_t obtainFileSize(const char *path) {
    struct stat buff;
    if(stat(path, &buff) < 0) {
        return 0;
    }
    return static_cast<size_t>(buff.st_size);
}