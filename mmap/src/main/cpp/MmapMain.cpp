//
// Created by haibo.wu on 2021/1/28.
//

#include "MmapMain.h"
#include "FileOption.h"
#include "interceptor/DefaultSerializer.h"

#include <iostream>
#include <fstream>

#define COMPRESS true

Buffer *MmapMain::createBuffer(const char *path) {
    auto *buffer = new Buffer();
    buffer->doInit(path);
    return buffer;
}

bool MmapMain::mmapWrite(Buffer *buffer, const char *content_chars) {
    if (!content_chars || !buffer || !buffer->isInit()) return false;
    int length = strlen(content_chars);

    //compress
    ByteBuffer compressedStr(64);
    int lengthAfterCompress;

    unsigned long long start, end;
    if(COMPRESS) {
        start = getTimeUSDNow();
        auto *tmp = new uint8_t[length];
        memcpy(tmp, content_chars, length);
        int codeCompress = compress(tmp, length,compressedStr, Z_BEST_COMPRESSION);
        delete [] tmp;
        end = getTimeUSDNow();
        LogTrace::Pair<LogTrace::ActionId, unsigned long long > zip(
                LogTrace::ActionId::zip, end -start);
        MmapMain::getTrace()->timeCostVector.push_back(zip);
        if(Z_OK != codeCompress) {
            LOGE("[mmap] compress failed");
            return false;
        }
        lengthAfterCompress = compressedStr.getSize();
        LOGI("[MapMain] length after compressed:%d", lengthAfterCompress);
    } else {
        compressedStr.append(reinterpret_cast<const uint8_t *>(content_chars), length);
        lengthAfterCompress = length;
    }
    start = getTimeUSDNow();
    HbLog log = LogProtocol::createLogItem(compressedStr.getData(),lengthAfterCompress);
    end = getTimeUSDNow();
    LogTrace::Pair<LogTrace::ActionId, unsigned long long > protocol(LogTrace::ActionId::protocol,end - start);
    MmapMain::getTrace()->timeCostVector.push_back(protocol);
    LOGD("return fro, createLogItem invoked, log addr:%d", &log);
    printLog(log);

    //serialize
    DefaultSerializer serializer;
    uint8_t *toSave = serializer.visit(&log);
    LOGW("[MapMain] all prepare done!");
    //save
    bool resultAppend = true;
    resultAppend = buffer->append(toSave, log.logLength);
//    delete [] toSave;
    return resultAppend;
}

bool MmapMain::flushAndArchive(Buffer *buffer) {
    if (!buffer) return false;
    buffer->onExit();
    delete buffer;
    return true;
}
