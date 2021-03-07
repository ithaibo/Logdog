//
// Created by Andy on 3/6/21.
//

#include <cstring>
#include "ByteBuffer.h"

ByteBuffer::ByteBuffer(uint32_t capacity) {
    this->data = new uint8_t [capacity];
    this->size = 0;
    this->position = 0;
}

ByteBuffer::~ByteBuffer() {
    delete [] this->data;
}

void ByteBuffer::append(const uint8_t *content, uint32_t len) {
    int offInput = 0;
    while (len > 0) {
        int lenCopy = remain() > len? len : remain();
        memcpy(this->data + this->position, content + offInput, lenCopy);
        len -= lenCopy;
        offInput += lenCopy;
        size += lenCopy;
        this->position += lenCopy;
        if (remain() <= 0) {
            expand();
        }
    }
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
    auto *temp = new uint8_t[this->position * 2];
    this->position *= 2;
    memcpy(temp, this->data, this->size);
    delete [] this->data;
    this->data = temp;
}

uint8_t *ByteBuffer::getData() {
    return this->data;
}
