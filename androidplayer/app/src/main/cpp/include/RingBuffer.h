// RingBuffer.h
#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <cstddef>
#include <cstdint>

class RingBuffer {
public:
    RingBuffer(size_t size);
    ~RingBuffer();

    size_t write(const uint8_t* data, size_t size);
    size_t read(uint8_t* data, size_t size);
    size_t availableRead();
    size_t availableWrite();
    size_t getChannelCount() const;
    size_t getBytesPerSample() const;
    void drop(size_t size);

private:
    uint8_t* buffer;
    size_t head;
    size_t tail;
    size_t maxSize;
    size_t channelCount;
    size_t bytesPerSample;
};

#endif // RINGBUFFER_H
