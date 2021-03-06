//
// Created by haibo.wu on 2021/1/28.
//

#include "MmapMain.h"
#include "FileOption.h"

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
    string compressedStr;
    int lengthAfterCompress;

    unsigned long long start, end;
    if(COMPRESS) {
        start = getTimeUSDNow();
        int codeCompress = compress((uint8_t *) content_chars, length, compressedStr,
                                    Z_BEST_COMPRESSION);
        end = getTimeUSDNow();
        LogTrace::Pair<LogTrace::ActionId, unsigned long long > zip(LogTrace::ActionId::zip, end -start);
        MmapMain::getTrace()->timeCostVector.push_back(zip);
        if(Z_OK != codeCompress) {
            LOGE("[mmap] compress failed");
            return false;
        }
        lengthAfterCompress = compressedStr.length();
    } else {
        compressedStr = string(content_chars);
        lengthAfterCompress = length;
    }
    start = getTimeUSDNow();
    HbLog log = LogProtocol::createLogItem(compressedStr, lengthAfterCompress);
    end = getTimeUSDNow();
    LogTrace::Pair<LogTrace::ActionId, unsigned long long > protocol(LogTrace::ActionId::protocol, end -start);
    MmapMain::getTrace()->timeCostVector.push_back(protocol);
    LOGD("return fro, createLogItem invoked, log addr:%d", &log);
    printLog(log);

    //serialize
    start = getTimeUSDNow();
    uint8_t *toSave = LogProtocol::serialize(log);
    end = getTimeUSDNow();
    LogTrace::Pair<LogTrace::ActionId, unsigned long long > serialize(LogTrace::ActionId::serialize, end -start);
    MmapMain::getTrace()->timeCostVector.push_back(serialize);
    //save
    bool resultAppend = buffer->append(toSave, log.logLength);
    //clear up
    delete []toSave;
    return resultAppend;
}

bool MmapMain::flushAndArchive(Buffer *buffer) {
    if (!buffer) return false;
    buffer->onExit();
    delete buffer;
    return true;
}
