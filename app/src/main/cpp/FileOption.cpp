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