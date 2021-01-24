#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <stdio.h>
#include <iostream>

#include "alog.h"
#include "FileOption.h"
#include "base64.h"
#include "Buffer.h"
#include <unistd.h>
#include "compress/Zip.h"
#include <zlib.h>

//TODO delete
#include "log_protocol.h"
#include "config.h"
#include "utils.h"
#include "log_type.h"

using namespace std;

static int registerNativeMethods(JNIEnv *env, jclass cls);

static Buffer *getBuffer(jlong addr);

extern "C" JNIEXPORT JNICALL jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }

    static const char *clsName = "com/andy/mmap/Mmap";
    jclass instance = env->FindClass(clsName);
    if (!instance) {
        LOGE("fail to locate class: %s", clsName);
        return -2;
    }

    int ret = registerNativeMethods(env, instance);
    if (0 != ret) {
        LOGE("fail to register native methods");
        return -3;
    }

    return JNI_VERSION_1_6;
}


static shared_ptr<HbLog> createLogItem(uint8_t *logContent, size_t lengthBody) {
    shared_ptr<HbLog> logMock(new HbLog());
    shared_ptr<LogBody> body(new LogBody());
    body->content = logContent;
    logMock->body = body.get();
    unsigned long crc = crc32(0L, body->content, lengthBody);
    logMock->header = createLogHeader(LogType::E2E, crc, nullptr, lengthBody).get();
    logMock->logLength =
            calculateHeaderLength() + logMock->header->otherLen + logMock->header->bodyLen;
    return logMock;
}


/**
 * 创建Buffer，并返回其地址
 * @param env
 * @param obj
 * @param jpath
 * @return
 */
static jlong createBuffer(JNIEnv *env, jobject obj, jstring jpath) {
    const char *path = env->GetStringUTFChars(jpath, JNI_FALSE);
    Buffer *buffer = new Buffer();
    buffer->doInit(path);
    env->ReleaseStringUTFChars(jpath, path);
    return (jlong) buffer;
}


/**
 * 将HbLog序列化为字节数组
 * @param log log
 * @return 字节数组
 */
static shared_ptr<uint8_t> serialize(const HbLog *log) {
    if (!log) return nullptr;
    assert(log->header != nullptr);
    assert(log->body != nullptr);

    shared_ptr<uint8_t> temp(new uint8_t[log->logLength]);
    uint32_t off = 0;
    memcpy(temp.get() + off, log->header->magic, LEN_HEADER_MAGIC);
    off += LEN_HEADER_MAGIC;
    memcpy(temp.get() + off, &log->header->headerLen, LEN_HEADER_HEADERLEN);
    off += LEN_HEADER_HEADERLEN;
    memcpy(temp.get() + off, &log->header->timestamp, LEN_HEADER_TIMESTAMP);
    off += LEN_HEADER_TIMESTAMP;
    memcpy(temp.get() + off, &log->header->version, LEN_HEADER_VERSION);
    off += LEN_HEADER_VERSION;
    memcpy(temp.get() + off, log->header->encrypt, LEN_HEADER_ENCRYPT);
    off += LEN_HEADER_ENCRYPT;
    memcpy(temp.get() + off, log->header->zip, LEN_HEADER_ZIP);
    off += LEN_HEADER_ZIP;
    memcpy(temp.get() + off, &log->header->type, LEN_HEADER_TYPE);
    off += LEN_HEADER_TYPE;
    memcpy(temp.get() + off, &log->header->crc32, LEN_HEADER_CRC32);
    off += LEN_HEADER_CRC32;
    memcpy(temp.get() + off, &log->header->otherLen, LEN_HEADER_OTHERLEN);
    off += LEN_HEADER_OTHERLEN;
    if (nullptr != log->header->other && log->header->otherLen > 0) {
        memcpy(temp.get() + off, log->header->other, log->header->otherLen);
        off += log->header->otherLen;
    }
    memcpy(temp.get() + off, &log->header->bodyLen, LEN_HEADER_BODYLEN);
    off += LEN_HEADER_BODYLEN;

    //body
    if (log->body->content) {
        memcpy(temp.get() + off, log->body->content, log->header->bodyLen);
    }

    return temp;
}

shared_ptr<HbLog> deserialize(const uint8_t *toParse) {
    shared_ptr<LogHeader> header(new LogHeader());
    // deserialize
    uint32_t off = 0;
    memcpy(header->magic, toParse + off, LEN_HEADER_MAGIC);
    off += LEN_HEADER_MAGIC;
    memcpy(&header->headerLen, toParse + off, LEN_HEADER_HEADERLEN);
    off += LEN_HEADER_HEADERLEN;
    memcpy(&header->timestamp, toParse + off, LEN_HEADER_TIMESTAMP);
    off += LEN_HEADER_TIMESTAMP;
    memcpy(&header->version, toParse + off, LEN_HEADER_VERSION);
    off += LEN_HEADER_VERSION;
    memcpy(header->encrypt, toParse + off, LEN_HEADER_ENCRYPT);
    off += LEN_HEADER_ENCRYPT;
    memcpy(header->zip, toParse + off, LEN_HEADER_ZIP);
    off += LEN_HEADER_ZIP;
    memcpy(&header->type, toParse + off, LEN_HEADER_TYPE);
    off += LEN_HEADER_TYPE;
    memcpy(&header->crc32, toParse + off, LEN_HEADER_CRC32);
    off += LEN_HEADER_CRC32;
    memcpy(&header->otherLen, toParse + off, LEN_HEADER_OTHERLEN);
    off += LEN_HEADER_OTHERLEN;
    if (header->otherLen > 0) {
        header->other = (uint8_t *) malloc(header->otherLen);
        memcpy(header->other, toParse + off, header->otherLen);
        off += header->otherLen;
    }
    memcpy(&header->bodyLen, toParse + off, LEN_HEADER_BODYLEN);
    off += LEN_HEADER_BODYLEN;

    shared_ptr<HbLog> parsedLog(new HbLog());
    parsedLog->header = header.get();

    if (header->bodyLen > 0) {
        std::string decompressLogBody;
        int codeDecompress = decompress(toParse + off, header->bodyLen, decompressLogBody);
        if (Z_OK == codeDecompress) {
            shared_ptr<LogBody> logBody(new LogBody());
            //body
            logBody->content = (uint8_t *) decompressLogBody.c_str();
            parsedLog->body = logBody.get();
        } else {
            LOGE("[mmap] decompress log body failed");
        }
    }

    return parsedLog;
}

static jboolean mmapWrite(JNIEnv *env, jobject thiz, jlong buffer, jstring content) {
    Buffer *bufferStatic = getBuffer(buffer);
    if (nullptr == bufferStatic || !bufferStatic->isInit()) {
        LOGW("buffer not init");
        return false;
    }

    const char *content_chars = env->GetStringUTFChars(content, JNI_FALSE);
    int length = strlen(content_chars);

    //compress
//    unsigned long long start = getTimeUSDNow();
    std::string compressedStr;
    int codeCompress = compress((uint8_t *) content_chars, length, compressedStr,
                                Z_DEFAULT_COMPRESSION);
//    unsigned long long endZip = getTimeUSDNow();
//    LOGI("[bridge] zip time cost:%lld", (endZip - start));

    if(Z_OK != codeCompress) {
        env->ReleaseStringUTFChars(content, content_chars);
        LOGE("[mmap] compress failed");
        return false;
    }
    int lengthAfterCompress = compressedStr.length();
//    LOGD("[mmap] after compress, code compress:%d, length:%d", codeCompress,lengthAfterCompress);

    shared_ptr<HbLog> log = createLogItem((uint8_t *) compressedStr.c_str(), lengthAfterCompress);
//    unsigned long long endCreateLog = getTimeUSDNow();
//    LOGI("[bridge] create log item time cost:%lld", (endCreateLog - endZip));
    printLog(log.get());

    // 将log的数据写入一个uint_8*中
    uint8_t *toSave = serialize(log.get()).get();
//    unsigned long long endSerialize = getTimeUSDNow();
//    LOGI("[bridge] serialize time cost:%lld", (endSerialize - endCreateLog));

    bool resultAppend = bufferStatic->append(toSave, log->logLength);
    env->ReleaseStringUTFChars(content, content_chars);
    if (!resultAppend) {
        return false;
    }
//    unsigned long long endSave = getTimeUSDNow();
//    LOGI("[bridge] save cost:%llu", (endSave - endSerialize));
//    LOGI("[bridge] all time cost:%llu", (endSave - start));
//    if (resultAppend) {
//        LOGI("[NativeLib] mmap write success");
//    } else {
//        LOGE("[NativeLib] mmap write fail");
//    }
    return true;
}


/**
 * 从指定的buffer中读取内容
 * @param env
 * @param thiz
 * @param buffer
 * @param path
 * @return
 */
static jstring readFile(JNIEnv *env, jobject thiz,
                        jlong buffer) {
    Buffer *bufferStatic = getBuffer(buffer);
    if (nullptr == bufferStatic) {
        return nullptr;
    }
    //read file content to buffer
    std::string *readFromFile = bufferStatic->getAll();
    if (nullptr == readFromFile) {
        return nullptr;
    }
    //TODO 这里可能存在多条日志
    shared_ptr<HbLog> parsedLog = deserialize((uint8_t *) readFromFile->c_str());
    printLog(parsedLog.get());
    jstring result = env->NewStringUTF((parsedLog->header->bodyLen <= 0)? "" : (const char *)parsedLog->body->content);
    //release memory
    delete readFromFile;
    return result;
}

static void onExit(JNIEnv *env, jobject thiz, jlong buffer) {
    Buffer *bufferStatic = getBuffer(buffer);
    if (nullptr == bufferStatic) return;
    bufferStatic->onExit();
    delete bufferStatic;
}


static JNINativeMethod methods[] = {
        {"createBuffer", "(Ljava/lang/String;)J",  (void *) createBuffer},
        {"mmapWrite",    "(JLjava/lang/String;)Z", (void *) mmapWrite},
        {"readFile",     "(J)Ljava/lang/String;",  (void *) readFile},
        {"onExit",       "(J)V",                   (void *) onExit}
};

static int registerNativeMethods(JNIEnv *env, jclass cls) {
    return env->RegisterNatives(cls,
                                methods,
                                sizeof(methods) / sizeof(methods[0]));
}

static Buffer *getBuffer(jlong addr) {
    return (Buffer *) addr;
}