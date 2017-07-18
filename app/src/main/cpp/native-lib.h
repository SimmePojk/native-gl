#ifndef GL_NATIVE_NATIVE_LIB_H
#define GL_NATIVE_NATIVE_LIB_H

#include <pthread.h>
#include <jni.h>
#include "Line.h"

extern "C"{
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnStart(JNIEnv *env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnResume(JNIEnv *env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnPause(JNIEnv *env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeOnStop(JNIEnv *env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeSetSurface(JNIEnv *env, jobject obj, jobject surface);
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeTouchStart(JNIEnv *env, jobject obj, float x, float y);
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeTouchMove(JNIEnv *env, jobject obj, float x, float y);
    JNIEXPORT void JNICALL Java_com_example_snan_gl_1native_MainActivity_nativeTouchEnd(JNIEnv *env, jobject obj, float x, float y);
};

#endif
