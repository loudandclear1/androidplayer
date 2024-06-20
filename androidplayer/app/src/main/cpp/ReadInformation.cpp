#include <jni.h>
#include <android/log.h>
extern "C" {
#include <libavformat/avformat.h>
}

#define LOG_TAG "FFmpegJNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_androidplayer_MainActivity_getVideoDuration(JNIEnv *env, jobject thiz, jstring file_path) {
    const char *input = env->GetStringUTFChars(file_path, 0);
    LOGD("File path: %s", input);

    AVFormatContext *formatContext = avformat_alloc_context();
    if (!formatContext) {
        LOGD("Failed to allocate format context");
        env->ReleaseStringUTFChars(file_path, input);
        return env->NewStringUTF("Failed to allocate format context");
    }

    if (avformat_open_input(&formatContext, input, NULL, NULL) != 0) {
        LOGD("Failed to open video file: %s", input);
        env->ReleaseStringUTFChars(file_path, input);
        return env->NewStringUTF("Failed to open video file");
    }

    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGD("Failed to retrieve video info");
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        env->ReleaseStringUTFChars(file_path, input);
        return env->NewStringUTF("Failed to retrieve video info");
    }

    int64_t duration = formatContext->duration;
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    env->ReleaseStringUTFChars(file_path, input);

    char durationStr[64];
    sprintf(durationStr, "%ld", duration / AV_TIME_BASE);

    LOGD("Video duration: %s seconds", durationStr);

    return env->NewStringUTF(durationStr);
}
