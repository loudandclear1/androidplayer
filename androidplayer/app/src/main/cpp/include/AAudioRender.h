#ifndef AAUDIO_RENDER_H
#define AAUDIO_RENDER_H

#include <aaudio/AAudio.h>
#include <cstring>
#include "RingBuffer.h"

class AAudioRender {
public:
    AAudioRender();
    ~AAudioRender();

    void configure(int32_t sampleRate, int32_t channelCnt, aaudio_format_t fmt);
    int start();
    int flush();
    int pause(bool p);
    void setCallback(AAudioStream_dataCallback callback, void *userData);

private:
    AAudioStream *stream;
    int32_t sample_rate;
    int32_t channel_count;
    aaudio_format_t format;
    AAudioStream_dataCallback callback;
    void *user_data;
    bool paused;
};

void setRingBuffer(RingBuffer* buffer);

#endif // AAUDIO_RENDER_H
