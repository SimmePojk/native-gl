#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/log.h>
#include <iostream>

#include "native-lib.h"
#include "renderer.h"

#define LOGGING

#ifdef LOGGING
#define LOG(...) __android_log_print(ANDROID_LOG_WARN, "Native", __VA_ARGS__)
#else
#define LOG(...)
#endif


static ANativeWindow *window = 0;
static Renderer *renderer = 0;

JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnStart(JNIEnv *env, jobject obj) {
    LOG("Native OnStart");
    renderer = new Renderer();
}


JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnResume(JNIEnv *env, jobject obj){
    //renderer->start();
}

JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnPause(JNIEnv *env, jobject obj){
    renderer->stop();
}

JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnStop(JNIEnv *env, jobject obj){
    delete renderer;
    renderer = 0;
}

JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeSetSurface(JNIEnv *env, jobject obj, jobject surface){
    if(surface){
        window = ANativeWindow_fromSurface(env, surface);
        renderer->setWindow(window);
        renderer->start();
    }
    else ANativeWindow_release(window);
}

JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeTouchStart(JNIEnv *env, jobject obj, float x, float y){
    x = x/(renderer->width>>1)-1;
    y = 1-y/(renderer->height>>1);
    renderer->activeLine = new Line((Point){x, y}, renderer->width, renderer->height);
}

JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeTouchMove(JNIEnv *env, jobject obj, float x, float y){
    x = x/(renderer->width>>1)-1;
    y = 1-y/(renderer->height>>1);
    renderer->activeLine->push((Point){x, y});
}

JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeTouchEnd(JNIEnv *env, jobject obj, float x, float y){
    x = x/(renderer->width>>1)-1;
    y = 1-y/(renderer->height>>1);
    renderer->activeLine->push((Point){x, y});
    Line *temp = renderer->activeLine;
    renderer->activeLine = 0;
    delete temp;
}
