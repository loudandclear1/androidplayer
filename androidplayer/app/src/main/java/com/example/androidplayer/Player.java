package com.example.androidplayer;

import android.util.Log;
import android.view.Surface;

public class Player {
    private long nativeContext;

    public enum PlayerState {
        None,
        Playing,
        Paused,
        End,
        Seeking
    }

    private Surface mSurface;
    private PlayerState mState = PlayerState.None;
    private String fileUri;

    public void setDataSource(String uri) {
        fileUri = uri;
    }

    public void setSurface(Surface surface) {
        mSurface = surface;
    }

    public void start() {
        nativePlay(fileUri, mSurface);
        mState = PlayerState.Playing;
    }

    public void pause(boolean p) {
        nativePause(p);
        if (p) {
            mState = PlayerState.Paused;
        } else {
            mState = PlayerState.Playing;
        }
    }

    public void stop() {
        nativeStop();
        mState = PlayerState.End;
    }

    public void seek(double position) {
        Log.d("seek", "seek position: " + position);
        nativeSeek(position);
    }

    public double getProgress() {
        double position = nativeGetPosition();
        double duration = getDuration();
        Log.d("Player", "Current position: " + position + ", Duration: " + duration);
        if (duration == 0) {
            return 0;
        }
        return position / duration;
    }

    public double getDuration() {
        return nativeGetDuration();
    }

    public PlayerState getState() {
        return mState;
    }

    public void setSpeed(float speed) {
        nativeSetSpeed(speed);
    }

    private native int nativePlay(String file, Surface surface);
    private native void nativePause(boolean p);
    private native int nativeSeek(double position);
    private native int nativeStop();
    private native int nativeSetSpeed(float speed);
    private native double nativeGetPosition();
    private native double nativeGetDuration();
}
