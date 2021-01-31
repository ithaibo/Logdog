//
// Created by haibo.wu on 2021/1/28.
//

#ifndef LOGDOG_MMAPMAIN_H
#define LOGDOG_MMAPMAIN_H

#include <cstring>
#include <android/log.h>
#include <cstdio>
#include <iostream>

#include "alog.h"
#include "FileOption.h"
#include "Buffer.h"
#include <unistd.h>
#include "compress/Zip.h"
#include <zlib.h>

#include "log_protocol.h"
#include "config.h"
#include "utils.h"
#include "log_type.h"
#include <vector>

using namespace std;



class MmapMain {
public:
    static Buffer *createBuffer(const char *path);

    static bool mmapWrite(Buffer *buffer, const char *content_chars);

    static bool flushAndArchive(Buffer *buffer);
};

#endif //LOGDOG_MMAPMAIN_H
