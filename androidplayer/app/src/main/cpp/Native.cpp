#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <aaudio/AAudio.h>
#include "RingBuffer.h"
#include "AAudioRender.h"
#include "ANWRender.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libswresample/swresample.h>
}

#define LOG_TAG "MyPlayer"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// 全局静态变量
static volatile bool isPaused = false;
static volatile bool isSeeking = false;
static volatile double seekPosition = 0.0;
static volatile double videoDuration = 0.0;
static volatile double currentPosition = 0.0;
static pthread_mutex_t videoMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t videoCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t audioCond = PTHREAD_COND_INITIALIZER;
static pthread_t readThread;
static pthread_t decodeThread;
static pthread_t renderThread;
static pthread_t audioPlayThread;
static JavaVM* javaVM = nullptr;

// 音视频同步
static volatile double videoClock = 0.0;
static volatile double audioClock = 0.0;
static std::mutex clockMutex;

// 安全队列实现
template<typename T>
class SafeQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(const T& value) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(value);
        cv.notify_all();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        T value = queue.front();
        queue.pop();
        return value;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(mtx);
        return queue.empty();
    }
};

// 全局队列
SafeQueue<AVPacket*> packetQueue;
SafeQueue<AVPacket*> audioPacketQueue;
SafeQueue<AVFrame*> frameQueue;

// 播放线程参数结构
struct PlayThreadParams {
    std::string filePath;
    jobject surface;
    jobject thiz;
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVCodecContext* audioCodecContext;
    int videoStreamIndex;
    int audioStreamIndex;
};

struct AudioPlayThreadParams {
    AVCodecContext* codecContext;
    int audioStreamIndex;
    AVFormatContext* formatContext;
};

aaudio_data_callback_result_t audioCallback(AAudioStream* stream, void* userData, void* audioData, int32_t numFrames) {
    if (isPaused) {
        memset(audioData, 0, numFrames * sizeof(int16_t) * 2);
        return AAUDIO_CALLBACK_RESULT_CONTINUE;
    }

    RingBuffer* ringBuffer = static_cast<RingBuffer*>(userData);
    ringBuffer->read(static_cast<uint8_t*>(audioData), numFrames * sizeof(int16_t) * 2);
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void* readThreadFunc(void* arg) {
    PlayThreadParams* params = (PlayThreadParams*)arg;
    AVPacket* packet = av_packet_alloc();

    while (true) {
        pthread_mutex_lock(&videoMutex);
        while (isPaused) {
            pthread_cond_wait(&videoCond, &videoMutex);
        }
        if (isSeeking) {
            av_seek_frame(params->formatContext, -1, (int64_t)(seekPosition / av_q2d(params->formatContext->streams[params->videoStreamIndex]->time_base)), AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(params->codecContext);
            avcodec_flush_buffers(params->audioCodecContext);
            isSeeking = false;
        }
        pthread_mutex_unlock(&videoMutex);

        if (av_read_frame(params->formatContext, packet) < 0) {
            LOGD("No more frames available");
            break;
        }

        if (packet->stream_index == params->videoStreamIndex) {
            packetQueue.push(packet);
            packet = av_packet_alloc(); // 分配新的包以供下一次读取
        } else if (packet->stream_index == params->audioStreamIndex) {
            audioPacketQueue.push(packet);
            packet = av_packet_alloc(); // 分配新的包以供下一次读取
        } else {
            av_packet_unref(packet);
        }
    }

    av_packet_free(&packet);
    return NULL;
}

void* decodeThreadFunc(void* arg) {
    PlayThreadParams* params = (PlayThreadParams*)arg;
    AVFrame* frame = av_frame_alloc();

    while (true) {
        pthread_mutex_lock(&videoMutex);
        while (isPaused) {
            pthread_cond_wait(&videoCond, &videoMutex);
        }
        pthread_mutex_unlock(&videoMutex);

        AVPacket* packet = packetQueue.pop();
        if (!packet) break;

        int response = avcodec_send_packet(params->codecContext, packet);
        if (response >= 0) {
            while (response >= 0) {
                response = avcodec_receive_frame(params->codecContext, frame);
                if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
                } else if (response < 0) {
                    char error_buf[128];
                    av_strerror(response, error_buf, sizeof(error_buf));
                    LOGD("Error while receiving a frame from the decoder: %s", error_buf);
                    return NULL;
                }
                frameQueue.push(frame);
                frame = av_frame_alloc(); // 分配新的帧以供下一次解码
            }
        }

        av_packet_unref(packet);
    }

    av_frame_free(&frame);
    return NULL;
}

void* renderThreadFunc(void* arg) {
    PlayThreadParams* params = (PlayThreadParams*)arg;
    JNIEnv* env;
    javaVM->AttachCurrentThread(&env, nullptr);

    ANativeWindow* nativeWindow = ANativeWindow_fromSurface(env, params->surface);
    ANativeWindow_setBuffersGeometry(nativeWindow, params->codecContext->width, params->codecContext->height, WINDOW_FORMAT_RGBA_8888);

    AVFrame* rgbFrame = av_frame_alloc();
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, params->codecContext->width, params->codecContext->height, 1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
    av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGBA, params->codecContext->width, params->codecContext->height, 1);

    SwsContext* sws_ctx = sws_getContext(params->codecContext->width, params->codecContext->height, params->codecContext->pix_fmt, params->codecContext->width, params->codecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, NULL, NULL, NULL);

    while (true) {
        pthread_mutex_lock(&videoMutex);
        while (isPaused) {
            pthread_cond_wait(&videoCond, &videoMutex);
        }
        pthread_mutex_unlock(&videoMutex);

        AVFrame* frame = frameQueue.pop();
        if (!frame) break;

        ANativeWindow_Buffer windowBuffer;
        ANativeWindow_lock(nativeWindow, &windowBuffer, NULL);
        sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0, params->codecContext->height, rgbFrame->data, rgbFrame->linesize);

        uint8_t* dst = (uint8_t*)windowBuffer.bits;
        int dstStride = windowBuffer.stride * 4;
        uint8_t* src = rgbFrame->data[0];
        int srcStride = rgbFrame->linesize[0];

        for (int h = 0; h < params->codecContext->height; h++) {
            memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
        }

        ANativeWindow_unlockAndPost(nativeWindow);

        double frameTime = (double)frame->pts * av_q2d(params->formatContext->streams[params->videoStreamIndex]->time_base);
        {
            std::unique_lock<std::mutex> lock(clockMutex);
            videoClock = frameTime;
        }
        double audioTime = currentPosition;
        double delay = frameTime - audioTime;

        if (delay > 0) {
            std::this_thread::sleep_for(std::chrono::duration<double>(delay));
        }

        currentPosition = frameTime;
        av_frame_free(&frame);
    }

    av_free(buffer);
    av_frame_free(&rgbFrame);
    ANativeWindow_release(nativeWindow);
    javaVM->DetachCurrentThread();
    return NULL;
}

void* audioDecodeThreadFunc(void* arg) {
    AudioPlayThreadParams* params = (AudioPlayThreadParams*)arg;
    AVCodecContext* codecContext = params->codecContext;
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    // 初始化一个 10 MB 大小的环形缓冲区
    RingBuffer ringBuffer(1024 * 1024 * 10);
    setRingBuffer(&ringBuffer);

    // 初始化 AAudioRender
    AAudioRender audioRender;
    int audioChannels = codecContext->ch_layout.nb_channels;
    audioRender.configure(codecContext->sample_rate, audioChannels, AAUDIO_FORMAT_PCM_I16); // 使用实际的通道数
    audioRender.setCallback(audioCallback, &ringBuffer);

    if (audioRender.start() != 0) {
        LOGD("Failed to start AAudioRender");
        av_packet_free(&packet);
        av_frame_free(&frame);
        return nullptr;
    }

    // 使用最新的 FFmpeg API 初始化重采样上下文
    SwrContext* swr_ctx = nullptr;
    AVChannelLayout out_ch_layout, in_ch_layout;
    av_channel_layout_default(&out_ch_layout, audioChannels); // 使用实际的通道数
    in_ch_layout = codecContext->ch_layout;

    swr_alloc_set_opts2(&swr_ctx,
                        &out_ch_layout, AV_SAMPLE_FMT_S16, codecContext->sample_rate,
                        &in_ch_layout, codecContext->sample_fmt, codecContext->sample_rate,
                        0, nullptr);
    swr_init(swr_ctx);

    // 分配输出缓冲区
    uint8_t* outputBuffer = (uint8_t*)av_malloc(2 * codecContext->sample_rate * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16));
    int outputBufferSize = 0;

    // 读取、解码和处理音频数据
    while (true) {
        pthread_mutex_lock(&audioMutex);
        while (isPaused) {
            pthread_cond_wait(&audioCond, &audioMutex);
        }
        pthread_mutex_unlock(&audioMutex);

        // 从队列中读取数据包
        AVPacket* packet = audioPacketQueue.pop();
        if (!packet) break;

        // 发送数据包到解码器
        if (avcodec_send_packet(codecContext, packet) >= 0) {
            // 从解码器接收帧
            while (avcodec_receive_frame(codecContext, frame) >= 0) {
                // 转换音频格式
                outputBufferSize = swr_convert(swr_ctx, &outputBuffer, 2 * codecContext->sample_rate,
                                               (const uint8_t**)frame->data, frame->nb_samples);
                if (outputBufferSize > 0) {
                    size_t dataSize = outputBufferSize * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * audioChannels;
                    ringBuffer.write(outputBuffer, dataSize); // 向环形缓冲区写入数据

                    double frameTime = (double)frame->pts * av_q2d(params->formatContext->streams[params->audioStreamIndex]->time_base);
                    {
                        std::unique_lock<std::mutex> lock(clockMutex);
                        audioClock = frameTime;
                    }

                    double videoTime;
                    {
                        std::unique_lock<std::mutex> lock(clockMutex);
                        videoTime = videoClock;
                    }

                    double delay = frameTime - videoTime;
                    if (delay > 0) {
                        std::this_thread::sleep_for(std::chrono::duration<double>(delay));
                    }
                }
            }
        }

        av_packet_unref(packet);
    }

    av_free(outputBuffer);
    swr_free(&swr_ctx);
    av_packet_free(&packet);
    av_frame_free(&frame);
    return nullptr;
}

extern "C" {

JNIEXPORT jint JNICALL
Java_com_example_androidplayer_Player_nativePlay(JNIEnv *env, jobject thiz, jstring file_path, jobject surface) {
    PlayThreadParams* params = new PlayThreadParams();
    env->GetJavaVM(&javaVM);
    params->filePath = env->GetStringUTFChars(file_path, NULL);
    params->surface = env->NewGlobalRef(surface);
    params->thiz = env->NewGlobalRef(thiz);

    params->formatContext = avformat_alloc_context();
    if (avformat_open_input(&params->formatContext, params->filePath.c_str(), NULL, NULL) != 0) {
        LOGD("Could not open file: %s", params->filePath.c_str());
        return -1;
    }

    if (avformat_find_stream_info(params->formatContext, NULL) < 0) {
        LOGD("Could not find stream information");
        avformat_close_input(&params->formatContext);
        return -1;
    }

    params->videoStreamIndex = -1;
    params->audioStreamIndex = -1;
    for (int i = 0; i < params->formatContext->nb_streams; i++) {
        if (params->formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            params->videoStreamIndex = i;
        } else if (params->formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            params->audioStreamIndex = i;
        }
    }

    if (params->videoStreamIndex == -1) {
        LOGD("Could not find video stream");
        avformat_close_input(&params->formatContext);
        return -1;
    }

    if (params->audioStreamIndex == -1) {
        LOGD("Could not find audio stream");
        avformat_close_input(&params->formatContext);
        return -1;
    }

    AVCodecParameters* videoCodecParameters = params->formatContext->streams[params->videoStreamIndex]->codecpar;
    AVCodec* videoCodec = const_cast<AVCodec *>(avcodec_find_decoder(
            videoCodecParameters->codec_id));
    if (videoCodec == NULL) {
        LOGD("Unsupported video codec!");
        avformat_close_input(&params->formatContext);
        return -1;
    }

    params->codecContext = avcodec_alloc_context3(videoCodec);
    if (!params->codecContext) {
        LOGD("Could not allocate video codec context");
        avformat_close_input(&params->formatContext);
        return -1;
    }

    if (avcodec_parameters_to_context(params->codecContext, videoCodecParameters) < 0) {
        LOGD("Could not copy video codec parameters to context");
        avcodec_free_context(&params->codecContext);
        avformat_close_input(&params->formatContext);
        return -1;
    }

    if (avcodec_open2(params->codecContext, videoCodec, NULL) < 0) {
        LOGD("Could not open video codec");
        avcodec_free_context(&params->codecContext);
        avformat_close_input(&params->formatContext);
        return -1;
    }

    AVCodecParameters* audioCodecParameters = params->formatContext->streams[params->audioStreamIndex]->codecpar;
    AVCodec* audioCodec = const_cast<AVCodec *>(avcodec_find_decoder(
            audioCodecParameters->codec_id));
    if (audioCodec == NULL) {
        LOGD("Unsupported audio codec!");
        avcodec_free_context(&params->codecContext);
        avformat_close_input(&params->formatContext);
        return -1;
    }

    AVCodecContext* audioCodecContext = avcodec_alloc_context3(audioCodec);
    if (!audioCodecContext) {
        LOGD("Could not allocate audio codec context");
        avcodec_free_context(&params->codecContext);
        avformat_close_input(&params->formatContext);
        return -1;
    }

    if (avcodec_parameters_to_context(audioCodecContext, audioCodecParameters) < 0) {
        LOGD("Could not copy audio codec parameters to context");
        avcodec_free_context(&audioCodecContext);
        avcodec_free_context(&params->codecContext);
        avformat_close_input(&params->formatContext);
        return -1;
    }

    if (avcodec_open2(audioCodecContext, audioCodec, NULL) < 0) {
        LOGD("Could not open audio codec");
        avcodec_free_context(&audioCodecContext);
        avcodec_free_context(&params->codecContext);
        avformat_close_input(&params->formatContext);
        return -1;
    }

    params->audioCodecContext = audioCodecContext;

    videoDuration = (double)params->formatContext->duration / AV_TIME_BASE;
    LOGD("Video duration: %f", videoDuration);

    AudioPlayThreadParams* audioParams = new AudioPlayThreadParams();
    audioParams->codecContext = audioCodecContext;
    audioParams->audioStreamIndex = params->audioStreamIndex;
    audioParams->formatContext = params->formatContext;

    pthread_create(&readThread, NULL, readThreadFunc, (void*)params);
    pthread_create(&decodeThread, NULL, decodeThreadFunc, (void*)params);
    pthread_create(&renderThread, NULL, renderThreadFunc, (void*)params);
    pthread_create(&audioPlayThread, NULL, audioDecodeThreadFunc, (void*)audioParams);

    return 0;
}

// JNI 暂停方法
JNIEXPORT void JNICALL
Java_com_example_androidplayer_Player_nativePause(JNIEnv *env, jobject thiz, jboolean pause) {
    LOGD("nativePause called with pause: %d", pause);
    pthread_mutex_lock(&videoMutex);
    pthread_mutex_lock(&audioMutex);
    isPaused = pause;
    if (!isPaused) {
        pthread_cond_broadcast(&videoCond);
        pthread_cond_broadcast(&audioCond);
    }
    pthread_mutex_unlock(&audioMutex);
    pthread_mutex_unlock(&videoMutex);
}

// JNI 拖动进度条方法
JNIEXPORT jint JNICALL
Java_com_example_androidplayer_Player_nativeSeek(JNIEnv *env, jobject thiz, jdouble position) {
    LOGD("nativeSeek called with position: %f", position);
    pthread_mutex_lock(&videoMutex);
    seekPosition = position * videoDuration;
    isSeeking = true;
    pthread_cond_broadcast(&videoCond);
    pthread_mutex_unlock(&videoMutex);
    return 0;
}

// JNI 停止方法
JNIEXPORT jint JNICALL
Java_com_example_androidplayer_Player_nativeStop(JNIEnv *env, jobject thiz) {
    LOGD("nativeStop called");
    packetQueue.push(nullptr); // 发送空包以通知队列结束
    audioPacketQueue.push(nullptr); // 发送空包以通知队列结束
    frameQueue.push(nullptr);  // 发送空帧以通知队列结束
    return 0;
}

// JNI 设置速度方法
JNIEXPORT jint JNICALL
Java_com_example_androidplayer_Player_nativeSetSpeed(JNIEnv *env, jobject thiz, jfloat speed) {
    LOGD("nativeSetSpeed called with speed: %f", speed);
    return 0;
}

// JNI 获取当前位置方法
JNIEXPORT jdouble JNICALL
Java_com_example_androidplayer_Player_nativeGetPosition(JNIEnv *env, jobject thiz) {
    LOGD("nativeGetPosition called");
    pthread_mutex_lock(&videoMutex);
    double position = currentPosition;
    pthread_mutex_unlock(&videoMutex);
    LOGD("Returning current position: %f", position);
    return position;
}

// JNI 获取持续时间方法
JNIEXPORT jdouble JNICALL
Java_com_example_androidplayer_Player_nativeGetDuration(JNIEnv *env, jobject thiz) {
    LOGD("nativeGetDuration called");
    return videoDuration;
}

// JNI OnLoad 方法
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    javaVM = vm;
    return JNI_VERSION_1_6;
}

}
