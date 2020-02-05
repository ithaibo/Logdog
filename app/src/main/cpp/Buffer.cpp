//
// Created by Andy on 2020-01-27.
//

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "Buffer.h"
#include "alog.h"

bool Buffer::mapMemory(const char *file_path) {
    //open file
    int fd = open(file_path,
            O_RDWR|O_CREAT|O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd == -1) {
        LOGE("[Buffer-mapMemory] open file failed");
        return false;
    }

    struct stat fileSize;
    if(fstat(fd, &fileSize) <0) {
        LOGE("[Buffer-mapMemory] get file size failed");
        return false;
    }
    if(fileSize.st_size < bufferSize) {
        if(ftruncate(fd, bufferSize) <0) {
            LOGE("[Buffer-mapMemory] set file size failed");
            return false;
        }
        LOGD("[Buffer-mapMemory] set file size success");
    }
    
    //map memory
    bufferInternal = (char*)mmap(nullptr,
            bufferSize,
            PROT_WRITE | PROT_READ,
            MAP_SHARED,
            fd,
            0);

    if(bufferInternal == MAP_FAILED) {
        LOGE("[Buffer-mapMemory]: mmap failed");
        return false;
    }
    //close file
    close(fd);
    //set off 0
    off = 0;

    return true;
}

off_t Buffer::append(const char *content) {
    //no offset, no append
    if(off <=0) return -1;
    size_t lengthToSave = strlen(content);
    if(lengthToSave > off) {
        //no enough offset, no append
        return -1;
    }
    size_t offNow = off;
    //copy content to buffer
    for(size_t index = offNow; index<offNow + lengthToSave; index++) {
        bufferInternal[index] = content[index - offNow];
    }
    //update off
    off+=lengthToSave;
    return offNow;
}

void Buffer::flush() {
    if(bufferInternal == nullptr)
        return;
    int unmapResult = munmap(bufferInternal, bufferSize/ sizeof(char));
    if(-1 == unmapResult) {
        LOGE("[unmap] error");
    }
}

char* Buffer::get(off_t start, size_t length) {
    if(off < start) return nullptr;
    if(length <=0) return nullptr;
    char* copyStr = new char[length+1];
    for(size_t index = 0; index < length; index++) {
        copyStr[index] = bufferInternal[index+start];
    }
    copyStr[length] = '\0';

    return copyStr;
}