// RingBuffer.cpp
#include "RingBuffer.h"
#include <cstring>

RingBuffer::RingBuffer(size_t size)
        : buffer(new uint8_t[size]), head(0), tail(0), maxSize(size), channelCount(2), bytesPerSample(2) {
    // Assume stereo and 16-bit sample as default
}

RingBuffer::~RingBuffer() {
    delete[] buffer;
}

size_t RingBuffer::write(const uint8_t* data, size_t size) {
    size_t bytesToWrite = size;
    if (availableWrite() < bytesToWrite) {
        bytesToWrite = availableWrite();
    }

    for (size_t i = 0; i < bytesToWrite; i++) {
        buffer[head] = data[i];
        head = (head + 1) % maxSize;
    }

    return bytesToWrite;
}

size_t RingBuffer::read(uint8_t* data, size_t size) {
    size_t bytesToRead = size;
    if (availableRead() < bytesToRead) {
        bytesToRead = availableRead();
    }

    for (size_t i = 0; i < bytesToRead; i++) {
        data[i] = buffer[tail];
        tail = (tail + 1) % maxSize;
    }

    return bytesToRead;
}

size_t RingBuffer::availableRead() {
    if (head >= tail) {
        return head - tail;
    } else {
        return maxSize - tail + head;
    }
}

size_t RingBuffer::availableWrite() {
    return maxSize - availableRead() - 1;
}

size_t RingBuffer::getChannelCount() const {
    return channelCount;
}

size_t RingBuffer::getBytesPerSample() const {
    return bytesPerSample;
}
