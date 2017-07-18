#include <pthread.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl31.h>
#include <android/log.h>

#include "renderer.h"

//#define LOGGING

#ifdef LOGGING
    #define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "Native", __VA_ARGS__)
#else
    #define LOG(...)
#endif

static const GLchar *vertexSource = "#version 300 es\n"
        "layout(location = 0) in vec2 position;"
        "void main(){"
        "   gl_Position = vec4(position, 0.0, 1.0);"
        "}\0";

static const GLchar *fragmentSource = "#version 300 es\n"
        "out vec4 FragColor;"
        "void main(){"
        "   FragColor = vec4(0.188, 0.43, 0.176, 1.0);"
        "}\0";

static const GLchar *texVertexSource = "#version 300 es\n"
        "layout(location = 0) in vec2 position;"
        "layout(location = 1) in vec2 texCoord;"
        "out vec2 TexCoord;"
        "void main(){"
        "   TexCoord = texCoord;"
        "   gl_Position = vec4(position, 0.0, 1.0);"
        "}\0";

static const GLchar *texFragmentSource = "#version 300 es\n"
        "in vec2 TexCoord;"
        "out vec4 FragColor;"
        "uniform sampler2D tex;"
        "void main(){"
        "   FragColor = texture(tex, TexCoord);"
        "}\0";

static const Point texData[] = {
        // Vertex coords   Texture coords
        { -1.0f,  1.0f }, { 0.0f, 2.0f },
        {  1.0f,  1.0f }, { 2.0f, 2.0f },
        { -1.0f, -1.0f }, { 0.0f, 0.0f },
        {  1.0f, -1.0f }, { 2.0f, 0.0f }
};

GLuint buildShader(const GLuint type, const GLchar *source){
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success){
        char log[512];
        glGetShaderInfoLog(shader, 512, 0, log);
        LOG("Shader Error: %s", log);
    }
    return shader;
}

Renderer::Renderer(): msg(MSG_NONE), display(0), context(0), activeLine(0){
    LOG("Creating renderer");
}

void Renderer::start(){
    LOG("Starting renderer");
    pthread_create(&threadId, 0, threadStartCallback, this);
}

void Renderer::stop(){
    LOG("Stopping renderer");
    msg = MSG_RENDER_LOOP_EXIT;
    pthread_join(threadId, 0);
}

void Renderer::setWindow(ANativeWindow *window){
    LOG("Setting window");
    this->window = window;
}

void Renderer::renderLoop(){
    bool rendering = true;
    LOG("Entering render loop");
    initialize();
    while(rendering){
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glUseProgram(program);
        glBindVertexArray(vao);
        if(activeLine) activeLine->draw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(texProgram);
        glBindVertexArray(texVao);
        glUniform1i(texHandle, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        eglSwapBuffers(display, surface);
        if(msg == MSG_RENDER_LOOP_EXIT){
            rendering = false;
            destroy();
        }
    }
}

bool Renderer::initialize(){
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;

    LOG("Initializing OpenGL context");

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, 0, 0);
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, window, 0);

    int attrib_list[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, attrib_list);
    eglMakeCurrent(display, surface, surface, context);

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    glDisable(GL_DITHER);
    glClearColor(0.9607f, 0.9607f, 0.9607f, 1.0f);

    glViewport(0, 0, width<<1, height<<1);

    GLuint vertexShader = buildShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = buildShader(GL_FRAGMENT_SHADER, fragmentSource);
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success){
        char log[512];
        glGetProgramInfoLog(program, 512, 0, log);
        LOG("Program Error %s", log);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    vertexShader = buildShader(GL_VERTEX_SHADER, texVertexSource);
    fragmentShader = buildShader(GL_FRAGMENT_SHADER, texFragmentSource);
    texProgram = glCreateProgram();
    glAttachShader(texProgram, vertexShader);
    glAttachShader(texProgram, fragmentShader);
    glLinkProgram(texProgram);
    glGetProgramiv(texProgram, GL_LINK_STATUS, &success);
    if(!success){
        char log[512];
        glGetProgramInfoLog(texProgram, 512, 0, log);
        LOG("Program Error %s", log);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    texHandle = (GLuint)glGetUniformLocation(texProgram, "tex");

    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &tex);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width<<1, height<<1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenVertexArrays(1, &texVao);
    glBindVertexArray(texVao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glGenBuffers(1, &texVbo);
    glBindBuffer(GL_ARRAY_BUFFER, texVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texData), texData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point)<<1, 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Point)<<1, (void*)sizeof(Point));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (NUM_POINTS<<3)*sizeof(Point), 0, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return true;
}

void Renderer::destroy(){
    LOG("Destroying renderer");
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &fbo);
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display, context);
    eglTerminate(display);
    display = EGL_NO_DISPLAY;
    surface = EGL_NO_SURFACE;
    context = EGL_NO_CONTEXT;
}

void* Renderer::threadStartCallback(void *self){
    Renderer *renderer = (Renderer*)self;
    renderer->renderLoop();
    pthread_exit(0);
}