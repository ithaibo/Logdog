//
// Created by haibo.wu on 2021/1/28.
//

#include "MmapMain.h"

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
    int codeCompress = compress((uint8_t *) content_chars, length, compressedStr,
                                Z_BEST_COMPRESSION);
    if(Z_OK != codeCompress) {
        LOGE("[mmap] compress failed");
        return false;
    }
    int lengthAfterCompress = compressedStr.length();
    HbLog log = LogProtocol::createLogItem(compressedStr, lengthAfterCompress);
    printLog(log);

    //serialize
    uint8_t *toSave = LogProtocol::serialize(log);
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
}
