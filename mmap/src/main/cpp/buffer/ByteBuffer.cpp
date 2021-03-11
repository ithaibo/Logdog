//
// Created by Andy on 3/6/21.
//

#include <cstring>
#include "ByteBuffer.h"

ByteBuffer::ByteBuffer(uint32_t capacity) {
    this->capacity = capacity;
    this->data = new uint8_t [capacity];
    this->size = 0;
    this->position = 0;
}

ByteBuffer::~ByteBuffer() {
    delete [] this->data;
}

void ByteBuffer::append(const uint8_t *content, uint32_t len) {
    uint8_t *ptr = data + size;
    const uint8_t *ptrInput = content;
    uint32_t lastLen = len;
    while (lastLen > 0) {
        int lenCopy = remain() > lastLen? lastLen : remain();
        memcpy(ptr, ptrInput, lenCopy);

        ptr += lenCopy;
        ptrInput += lenCopy;
        size += lenCopy;
        lastLen -= lenCopy;

        if (remain() <= 0) {
            expand();
            ptr = data + size;
        }
    }
    position = size;
}

uint32_t ByteBuffer::getCapacity() {
    return this->capacity;
}

uint32_t ByteBuffer::getSize() {
    return this->size;
}

uint32_t ByteBuffer::getPosition() {
    return this->position;
}

void ByteBuffer::setPosition(uint32_t position) {
    if (position > this->capacity) throw "buffer overflow!!";
    this->position = position;
}

uint32_t ByteBuffer::remain() {
    return this->capacity - this->size;
}

void ByteBuffer::expand() {
    this->capacity *= 2;
    auto *temp = new uint8_t[this->capacity];
    memcpy(temp, this->data, this->size);
    delete [] this->data;
    this->data = nullptr;
    this->data = temp;
}

uint8_t *ByteBuffer::getData() {
    return this->data;
}
