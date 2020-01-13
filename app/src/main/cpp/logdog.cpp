//
// Created by haibo.wu on 2020/1/13.
//
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "logdog.h"
#include "alog.h"




void MmapWrite(char const* filePath, char const* toSave) {
    int fd = open(filePath, O_RDWR | O_CREAT | O_TRUNC);
    if(fd == -1) {
        LOGE("[mmap]: file, path: %s open failed, reason: %s",
                filePath,
                strerror(errno));
        return;
    }


    //get page size
    size_t pageSize = getpagesize();
    LOGD("[mmap]: page size: %lu", pageSize);


    //do mmap
    char* buffer;
    buffer = (char*)mmap(0, pageSize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if(NULL == buffer) {
        LOGE("[mmap]: buffer pointer is null");
        return;
    }

    if(buffer == MAP_FAILED) {
        LOGE("[mmap]: mmap failed, reason: %s",strerror(errno));
        return;
    }


    size_t length = strlen(toSave);
    LOGD("[mmap]: content to save: %s", toSave);
    LOGD("[mmap]: length of toSave: %lu", length);

//    fixme copy memory
//      memcpy(buffer, toSave, 20);
//      buffer[0] = 'A';

    LOGD("[mmap]: memory copy done");
    munmap(buffer, pageSize);
}
