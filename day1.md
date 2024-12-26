

### 1.构建项目

在构建项目中遇到AGP版本不兼容问题，如图所示

![构建失败](../image/build_fail.png)

通过修改libs.versions.toml中AGP的版本后构建成功

![构建成功](../XIAOMI/project/day1/image/build_success.png)



### 2.集成ffmpeg

在ffmpeg源代码目录下，执行脚本，集成ffmpeg到Android

arm64-v8a –已被弃用

```shell
#!/bin/bash

FFmpeg_SOURCE=/home/hgz/Desktop/ffmpeg-7.0.1
ANDROID_NDK=/home/hgz/android-ndk-r26d
API=21

ARCH=aarch64
CPU=armv8-a
CROSS_PREFIX=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$API-
SYSROOT=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot
TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin
PREFIX=/home/hgz/Desktop/ffmpeg-7.0.1/build
ARCH_ROOT=/home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/jniLibs/arm64-v8a

cd $FFmpeg_SOURCE

./configure \
    --prefix=$PREFIX \
    --enable-static \
    --disable-doc \
    --enable-shared \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
     --enable-avdevice \
    --enable-avfilter \
    --enable-postproc \
    --enable-avformat \
    --enable-avcodec \
    --enable-avutil \
    --enable-swresample \
    --enable-swscale \
    --enable-symver \
    --cross-prefix=$CROSS_PREFIX \
    --target-os=android \
    --arch=$ARCH \
    --cpu=$CPU \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-fPIC -O3" \
    --extra-cxxflags="-fPIC -O3" \
    --cc=$CC \
    --cxx=$CXX \
    --nm=$TOOLCHAIN/llvm-nm \
    --strip=$TOOLCHAIN/llvm-strip \
    --ranlib=$TOOLCHAIN/llvm-ranlib \
    --ar=$TOOLCHAIN/llvm-ar


make -j 12
make install

cp -r $FFmpeg_SOURCE/build/include $ARCH_ROOT
$TOOLCHAIN/aarch64-linux-android$API-clang  -shared -fPIC -o $ARCH_ROOT/libffmpeg-huguozheng.so \
    $FFmpeg_SOURCE/build/lib/libavcodec.a \
    $FFmpeg_SOURCE/build/lib/libavdevice.a \
    $FFmpeg_SOURCE/build/lib/libavfilter.a \
    $FFmpeg_SOURCE/build/lib/libavformat.a \
    $FFmpeg_SOURCE/build/lib/libavutil.a \
    $FFmpeg_SOURCE/build/lib/libswresample.a \
    $FFmpeg_SOURCE/build/lib/libswscale.a \
     -Wl,--whole-archive -landroid -llog -lz -lm -ldl -pthread -Wl,--no-whole-archive

```



armeabi-v7a –已被弃用

```shell
#!/bin/bash

FFmpeg_SOURCE=/home/hgz/Desktop/ffmpeg-7.0.1
ANDROID_NDK=/home/hgz/android-ndk-r26d
API=21

ARCH=armeabi-v7a
CPU=armv7-a
CROSS_PREFIX=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$API-
SYSROOT=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot
TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin
PREFIX=/home/hgz/Desktop/ffmpeg-7.0.1/build
ARCH_ROOT=/home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/jniLibs/$ARCH

cd $FFmpeg_SOURCE

export CC=$TOOLCHAIN/armv7a-linux-androideabi$API-clang
export CXX=$TOOLCHAIN/armv7a-linux-androideabi$API-clang++

./configure \
    --prefix=$PREFIX \
    --enable-static \
    --disable-doc \
    --enable-shared \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
     --enable-avdevice \
    --enable-avfilter \
    --enable-postproc \
    --enable-avformat \
    --enable-avcodec \
    --enable-avutil \
    --enable-swresample \
    --enable-swscale \
    --enable-symver \
    --cross-prefix=$CROSS_PREFIX \
    --target-os=android \
    --arch=$ARCH \
    --cpu=$CPU \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-fPIC -O3" \
    --extra-cxxflags="-fPIC -O3" \
    --cc=$CC \
    --cxx=$CXX \
    --nm=$TOOLCHAIN/llvm-nm \
    --strip=$TOOLCHAIN/llvm-strip \
    --ranlib=$TOOLCHAIN/llvm-ranlib \
    --ar=$TOOLCHAIN/llvm-ar


make -j 12
make install

cp -r $FFmpeg_SOURCE/build/include $ARCH_ROOT
$TOOLCHAIN/armv7a-linux-androideabi$API-clang  -shared -fPIC -o $ARCH_ROOT/libffmpeg-huguozheng.so \
    $FFmpeg_SOURCE/build/lib/libavcodec.a \
    $FFmpeg_SOURCE/build/lib/libavdevice.a \
    $FFmpeg_SOURCE/build/lib/libavfilter.a \
    $FFmpeg_SOURCE/build/lib/libavformat.a \
    $FFmpeg_SOURCE/build/lib/libavutil.a \
    $FFmpeg_SOURCE/build/lib/libswresample.a \
    $FFmpeg_SOURCE/build/lib/libswscale.a \
     -Wl,--whole-archive -landroid -llog -lz -lm -ldl -pthread -Wl,--no-whole-archive

```



由于我使用的是虚拟设备，所以还需要编译x86_64库 –已被弃用

```shell
#!/bin/bash

FFmpeg_SOURCE=/home/hgz/Desktop/ffmpeg-7.0.1
ANDROID_NDK=/home/hgz/android-ndk-r26d
API=21

ARCH=x86_64
CPU=x86-64
CROSS_PREFIX=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android$API-
SYSROOT=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot
TOOLCHAIN=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin
PREFIX=/home/hgz/Desktop/ffmpeg-7.0.1/build
ARCH_ROOT=/home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/jniLibs/$ARCH

cd $FFmpeg_SOURCE

export CC=$TOOLCHAIN/x86_64-linux-android$API-clang
export CXX=$TOOLCHAIN/x86_64-linux-android$API-clang++

./configure \
    --prefix=$PREFIX \
    --enable-static \
    --disable-doc \
    --enable-shared \
    --disable-ffmpeg \
    --disable-ffplay \
    --disable-ffprobe \
     --enable-avdevice \
    --enable-avfilter \
    --enable-postproc \
    --enable-avformat \
    --enable-avcodec \
    --enable-avutil \
    --enable-swresample \
    --enable-swscale \
    --enable-symver \
    --cross-prefix=$CROSS_PREFIX \
    --target-os=android \
    --arch=$ARCH \
    --cpu=$CPU \
    --enable-cross-compile \
    --sysroot=$SYSROOT \
    --extra-cflags="-fPIC -O3" \
    --extra-cxxflags="-fPIC -O3" \
    --cc=$CC \
    --cxx=$CXX \
    --nm=$TOOLCHAIN/llvm-nm \
    --strip=$TOOLCHAIN/llvm-strip \
    --ranlib=$TOOLCHAIN/llvm-ranlib \
    --ar=$TOOLCHAIN/llvm-ar


make -j 12
make install

cp -r $FFmpeg_SOURCE/build/include $ARCH_ROOT
$TOOLCHAIN/x86_64-linux-android$API-clang -shared -fPIC -o $ARCH_ROOT/libffmpeg-huguozheng.so \
    $FFmpeg_SOURCE/build/lib/libavcodec.a \
    $FFmpeg_SOURCE/build/lib/libavdevice.a \
    $FFmpeg_SOURCE/build/lib/libavfilter.a \
    $FFmpeg_SOURCE/build/lib/libavformat.a \
    $FFmpeg_SOURCE/build/lib/libavutil.a \
    $FFmpeg_SOURCE/build/lib/libswresample.a \
    $FFmpeg_SOURCE/build/lib/libswscale.a \
     -Wl,--whole-archive -landroid -llog -lz -lm -ldl -pthread -Wl,--no-whole-archive
```

接下来，修改CMakeLists.txt中需要的动态库名称，对应我的ffmpeg库文件

![修改需要库名称](../XIAOMI/project/day1/image/change_libname.png)

运行项目，可以看到运行成功

![运行成功](../XIAOMI/project/day1/image/successs_result.png)

一直没有成功，不知道为什么

![fail](../XIAOMI/project/day1/image/fail.png)



#### 2024年6月13日，删项目重写

更改sdk路径，还是老样子修改AGP版本

![sdk](../XIAOMI/project/day1/image/sdk_path.png)

因为Android只能加载*.so结尾的动态库，不能识别如*.so.57结尾的文件，所以这里要修改configure

![before](../XIAOMI/project/day1/image/fix_config.png)

修改为

![after](../XIAOMI/project/day1/image/after_fix_config.png)

进行下一步执行修改后的configure文件

![next1](../XIAOMI/project/day1/image/next1.png)

![next1_result](../XIAOMI/project/day1/image/next1_result.png)

![next1_result1](../XIAOMI/project/day1/image/next1_result1.png)



接下来编写脚本，把ffmpeg集成到Android中，之前的脚本作废了 –正在使用

```shell
#!/bin/bash
NDK_HOME=/home/hgz/android-ndk-r26d

 build_one()
{

./configure \
--ln_s="cp -rf" \
--prefix=$PREFIX \
--cc=$TOOLCHAIN/$TARGET$API-clang \
--cxx=$TOOLCHAIN/$TARGET$API-clang++ \
--ld=$TOOLCHAIN/$TARGET$API-clang \
--nm=$NM \
--strip=$STRIP \
--target-os=linux \
--arch=$ARCH \
--cpu=$CPU \
--cross-prefix=$CROSS_PREFIX \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--enable-cross-compile \
--disable-doc \
--enable-shared \
--disable-static \
--disable-asm \
--disable-symver \
--enable-small \
--enable-gpl \
--enable-avdevice \
--enable-avfilter \
--enable-postproc \
--enable-avformat \
--enable-avcodec \
--enable-avutil \
--enable-swresample \
--enable-swscale \
--enable-runtime-cpudetect \
--extra-cflags="-Os -fpic $CFLAGS" \
--extra-ldflags="$LDFLAGS"


make clean
make -j64
make install
}

#armeabi-v7a
API=21
CPU=armv7-a
ARCH=armv7a
PLATFORM=armeabi-v7a
TARGET=armv7a-linux-androideabi
TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
SYSROOT=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/$TARGET$API-
NM=$TOOLCHAIN/llvm-nm
STRIP=$TOOLCHAIN/llvm-strip
PREFIX=/home/hgz/Desktop/android/$PLATFORM
CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -mfpu=neon"
LDFLAGS="-march=armv7-a -Wl,--fix-cortex-a8"
build_one
cp $PREFIX/lib/*-*.so /home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/jniLibs/$PLATFORM

#arm64
API=21
CPU=armv8-a
ARCH=aarch64
PLATFORM=arm64-v8a
TARGET=aarch64-linux-android
TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
SYSROOT=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/$TARGET$API-
NM=$TOOLCHAIN/llvm-nm
STRIP=$TOOLCHAIN/llvm-strip
PREFIX=/home/hgz/Desktop/android/$PLATFORM
CFLAGS=""
LDFLAGS=""
build_one
cp $PREFIX/lib/*-*.so /home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/jniLibs/$PLATFORM


#x86_64
API=21
CPU=x86-64
ARCH=x86_64
PLATFORM=x86_64
TARGET=x86_64-linux-android
TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
SYSROOT=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/$TARGET$API-
NM=$TOOLCHAIN/llvm-nm
STRIP=$TOOLCHAIN/llvm-strip
PREFIX=/home/hgz/Desktop/android/$PLATFORM
CFLAGS="-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=haswell"
LDFLAGS=""
build_one
cp $PREFIX/lib/*-*.so /home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/jniLibs/$PLATFORM
cp -r $PREFIX/include/* /home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/cpp/include
```

编写读取视频信息的cpp文件，配置CmakeLists.txt，在MainActivity.java中调用cpp 详细见源文件，这里不再赘述

这里在debug启动app之前还要修改AndroidManifest.xml，让我们有权限访问sdcard下的1.mp4视频

添加这三行

```xml
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
    <uses-permission android:name="android.permission.MANAGE_EXTERNAL_STORAGE" />
```

运行效果，终于打印出来视频的时长了

![打印视频时长](../XIAOMI/project/day1/image/video_duration.png)

为了验证结果的准确性，我特意去查了视频的信息

![视频信息](../XIAOMI/project/day1/image/video_information.png)

可以看到输出的信息是准确的

完整的测试结果

![完整结果](../XIAOMI/project/day1/image/result1.png)



更新脚本 –已被弃用

```she
#!/bin/bash
NDK_HOME=/home/hgz/android-ndk-r26d

 build_one()
{

./configure \
--ln_s="cp -rf" \
--prefix=$PREFIX \
--cc=$TOOLCHAIN/$TARGET$API-clang \
--cxx=$TOOLCHAIN/$TARGET$API-clang++ \
--ld=$TOOLCHAIN/$TARGET$API-clang \
--nm=$NM \
--ar=$TOOLCHAIN/llvm-ar \
--ranlib=$TOOLCHAIN/llvm-ranlib \
--strip=$STRIP \
--target-os=android \
--arch=$ARCH \
--cpu=$CPU \
--cross-prefix=$CROSS_PREFIX \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--enable-cross-compile \
--disable-doc \
--disable-shared \
--enable-static \
--disable-asm \
--disable-symver \
--enable-small \
--enable-gpl \
--enable-avdevice \
--enable-avfilter \
--enable-postproc \
--enable-avformat \
--enable-avcodec \
--enable-avutil \
--enable-swresample \
--enable-swscale \
--enable-runtime-cpudetect \
--extra-cflags="-Os -fpic $CFLAGS" \
--extra-ldflags="$LDFLAGS"


make clean
make -j64
make install
}

create_combined_dynamic_lib() {
    echo "Creating combined dynamic library..."

    local static_libs=(
        "$PREFIX/lib/libavutil.a"
        "$PREFIX/lib/libswresample.a"
        "$PREFIX/lib/libavformat.a"
        "$PREFIX/lib/libavfilter.a"
        "$PREFIX/lib/libavdevice.a"
        "$PREFIX/lib/libavcodec.a"
    )

    local output_lib="/home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/jniLibs/$PLATFORM/libffmpeg-huguozheng.so"

    $TOOLCHAIN/$TARGET$API-clang -shared -o $output_lib \
        -Wl,--whole-archive ${static_libs[@]} -Wl,--no-whole-archive \
        -Wl,--enable-new-dtags \
        -Wl,-rpath-link=$SYSROOT/usr/lib \
        -Wl,-rpath-link=$PREFIX/lib \
        -L$SYSROOT/usr/lib \
        -L$PREFIX/lib \
        $LDFLAGS

    echo "Combined dynamic library created at: $output_lib"
}



#armeabi-v7a
API=21
CPU=armv7-a
ARCH=armv7a
PLATFORM=armeabi-v7a
TARGET=armv7a-linux-androideabi
TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
SYSROOT=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/$TARGET$API-
NM=$TOOLCHAIN/llvm-nm
STRIP=$TOOLCHAIN/llvm-strip
PREFIX=/home/hgz/Desktop/android/$PLATFORM
CFLAGS="-march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -mfpu=neon"
LDFLAGS="-march=armv7-a -Wl,--fix-cortex-a8"
build_one
create_combined_dynamic_lib 

#arm64
API=21
CPU=armv8-a
ARCH=aarch64
PLATFORM=arm64-v8a
TARGET=aarch64-linux-android
TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
SYSROOT=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/$TARGET$API-
NM=$TOOLCHAIN/llvm-nm
STRIP=$TOOLCHAIN/llvm-strip
PREFIX=/home/hgz/Desktop/android/$PLATFORM
CFLAGS=""
LDFLAGS=""
build_one
create_combined_dynamic_lib 


#x86_64
API=21
CPU=x86-64
ARCH=x86_64
PLATFORM=x86_64
TARGET=x86_64-linux-android
TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin
SYSROOT=$NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot
CROSS_PREFIX=$TOOLCHAIN/$TARGET$API-
NM=$TOOLCHAIN/llvm-nm
STRIP=$TOOLCHAIN/llvm-strip
PREFIX=/home/hgz/Desktop/android/$PLATFORM
CFLAGS="-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=haswell"
LDFLAGS="-latomic"
build_one
create_combined_dynamic_lib 
cp -r $PREFIX/include/* /home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/cpp/include
```

不知道为什么

```problem
FATAL EXCEPTION: main
                                                                                                    Process: com.example.androidplayer, PID: 17519
                                                                                                    java.lang.UnsatisfiedLinkError: dlopen failed: library "/home/hgz/Desktop/XIAOMI/project/day1/androidplayer/app/src/main/cpp/../jniLibs/x86_64/libffmpeg-huguozheng.so" not found: needed by /data/app/~~lLodsCehCS4TVWa9IfypSw==/com.example.androidplayer-e-9KaLLDNDpW7Yr0v5VYDQ==/base.apk!/lib/x86_64/libandroidplayer.so in namespace classloader-namespace
                                                                                                    	at java.lang.Runtime.loadLibrary0(Runtime.java:1077)
                                                                                                    	at java.lang.Runtime.loadLibrary0(Runtime.java:998)
                                                                                                    	at java.lang.System.loadLibrary(System.java:1661)
                                                                                                    	at com.example.androidplayer.MainActivity.<clinit>(MainActivity.java:28)
                                                                                                    	at java.lang.Class.newInstance(Native Method)
                                                                                                    	at android.app.AppComponentFactory.instantiateActivity(AppComponentFactory.java:95)
                                                                                                    	at androidx.core.app.CoreComponentFactory.instantiateActivity(CoreComponentFactory.java:45)
                                                                                                    	at android.app.Instrumentation.newActivity(Instrumentation.java:1339)
                                                                                                    	at android.app.ActivityThread.performLaunchActivity(ActivityThread.java:3538)
                                                                                                    	at android.app.ActivityThread.handleLaunchActivity(ActivityThread.java:3782)
                                                                                                    	at android.app.servertransaction.LaunchActivityItem.execute(LaunchActivityItem.java:101)
                                                                                                    	at android.app.servertransaction.TransactionExecutor.executeCallbacks(TransactionExecutor.java:135)
                                                                                                    	at android.app.servertransaction.TransactionExecutor.execute(TransactionExecutor.java:95)
                                                                                                    	at android.app.ActivityThread$H.handleMessage(ActivityThread.java:2307)
                                                                                                    	at android.os.Handler.dispatchMessage(Handler.java:106)
                                                                                                    	at android.os.Looper.loopOnce(Looper.java:201)
                                                                                                    	at android.os.Looper.loop(Looper.java:288)
                                                                                                    	at android.app.ActivityThread.main(ActivityThread.java:7872)
                                                                                                    	at java.lang.reflect.Method.invoke(Native Method)
                                                                                                    	at com.android.internal.os.RuntimeInit$MethodAndArgsCaller.run(RuntimeInit.java:548)
                                                                                                    	at com.android.internal.os.ZygoteInit.main(ZygoteInit.java:936)
```

我不知道第一点要求到底是什么意思，把所有库集成到一个库中libffmpeg-huguozheng.so？这样的意义是什么，是我理解错了？

我选择代码回滚，直接将动态库拷贝到对应的架构文件夹下
