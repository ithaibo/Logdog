//
// Created by Andy on 3/6/21.
//

#ifndef LOGDOG_BYTEBUFFER_H
#define LOGDOG_BYTEBUFFER_H


#include <cstdint>

/**
 * 可自动扩容的数据缓冲区。扩容方式：*2
 */
class ByteBuffer {
private:
    /**
     * 容量
     */
    uint32_t capacity;
    /**
     * 数据的当前读写位置
     */
    uint32_t position;
    /**
     * 数据的长度
     */
    uint32_t size;
    /**
     * 存放数据的动态数组
     */
    uint8_t *data = nullptr;
    /**
     *
     * @return
     */
    uint32_t remain();

    void expand();

public:
    ByteBuffer(uint32_t capacity);
    ~ByteBuffer();

    /**
     * 写入数据
     * @param content 数据内容
     * @param len 数据长度
     */
    void append(const uint8_t *content, uint32_t len);
    /**
     * 获取容量
     * @return 容量
     */
    uint32_t getCapacity();
    /**
     * 获取数据长度
     * @return 数据长度
     */
    uint32_t getSize();
    /**
     * 获取当前读写的位置
     * @return 位置
     */
    uint32_t getPosition();
    /**
     * 设置读写位置
     * @param position 读写位置
     */
    void setPosition(uint32_t position);

    uint8_t *getData();
};


#endif //LOGDOG_BYTEBUFFER_H
