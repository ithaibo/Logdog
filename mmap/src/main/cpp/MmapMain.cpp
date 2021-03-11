//
// Created by haibo.wu on 2021/1/28.
//

#include "MmapMain.h"
#include "FileOption.h"
#include "interceptor/DefaultSerializer.h"

#include <iostream>
#include <fstream>


Buffer *MmapMain::createBuffer(const char *path) {
    auto *buffer = new Buffer();
    buffer->doInit(path);
    return buffer;
}

bool MmapMain::mmapWrite(Buffer *buffer, const char *content_chars) {
    if (!content_chars || !buffer || !buffer->isInit()) return false;
    int length = strlen(content_chars);


    //serialize
    HbLog log = LogProtocol::createLogItem((uint8_t *) content_chars, length);
    DefaultSerializer serializer;
    uint8_t *serialized = serializer.visit(&log);
    delete [] log.body.content;
    LOGD("[Main] serialize done.");

    //compress
    LOGD("[main] start to compress, input length:%d", log.logLength);
    unsigned long long start, end;
    start = getTimeUSDNow();
    ByteBuffer compressedBuffer(64);
    int lengthAfterCompress;
    int codeCompress = compress(serialized, log.logLength, compressedBuffer, Z_BEST_COMPRESSION);
    delete [] serialized;
    if (Z_OK != codeCompress) {
        LOGE("[mmap] compress failed");
        return false;
    }
    end = getTimeUSDNow();
    LogTrace::Pair<LogTrace::ActionId, unsigned long long> zip(LogTrace::ActionId::zip, end - start);
    MmapMain::getTrace()->timeCostVector.push_back(zip);
    lengthAfterCompress = compressedBuffer.getSize();
    LOGI("[MapMain] length after compressed:%d", lengthAfterCompress);

    //TODO encrypt
    LOGD("[main] encrypt....");

    //save
    bool resultAppend = true;
    resultAppend = buffer->append(compressedBuffer.getData(), log.logLength);
//    delete [] toSave;
    return resultAppend;
}

bool MmapMain::flushAndArchive(Buffer *buffer) {
    if (!buffer) return false;
    buffer->onExit();
    delete buffer;
    return true;
}
