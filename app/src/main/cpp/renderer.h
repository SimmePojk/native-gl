#ifndef GL_NATIVE_RENDERER_H
#define GL_NATIVE_RENDERER_H

#include <pthread.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "Line.h"

class Renderer{
    enum RenderThreadMessage{
        MSG_NONE = 0,
        MSG_RENDER_LOOP_EXIT
    };

    pthread_t threadId;
    enum RenderThreadMessage msg;

    ANativeWindow *window;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    GLuint vao;
    GLuint vbo;

    GLuint texVao;
    GLuint texVbo;

    GLuint fbo;
    GLuint tex;

    GLuint program;
    GLuint texProgram;
    GLuint texHandle;

    void renderLoop();
    bool initialize();
    void destroy();
    static void* threadStartCallback(void *self);

public:

    EGLint width;
    EGLint height;

    Line *activeLine;

    Renderer();
    void start();
    void stop();
    void setWindow(ANativeWindow *window);
};

#endif
