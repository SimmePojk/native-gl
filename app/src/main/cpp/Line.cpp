#include <GLES3/gl3.h>
#include <cmath>
#include <android/log.h>

#include "Line.h"
#include "native-lib.h"

//#define LOGGING

#ifdef LOGGING
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "Native", __VA_ARGS__)
#else
#define LOG(...)
#endif

void LineBuffer::expand(){
    Point *old = vertices;
    cap <<= 1;
    vertices = new Point[cap];
    for(uint i=0; i<size; ++i) vertices[i] = old[i];
    delete[] old;
}

LineBuffer::LineBuffer(){
    vertices = new Point[64*NUM_POINTS];
    cap = 64*NUM_POINTS;
    size = 0;
}

void LineBuffer::push(Point *points, Point startNorm){
    Point norm = startNorm;
    Point offset;
    float len;
    for(uint i=0; i<NUM_POINTS; ++i){
        if(i != 0){
            norm = { points[i-1].y-points[i+1].y, points[i+1].x-points[i-1].x };
            len = (float)sqrt(norm.x*norm.x+norm.y*norm.y);
            norm = norm/len;
        }
        offset = { 3.0f*norm.x/width, 3.0f*norm.y/height };
        vertices[size++] = points[i]+offset;
        vertices[size++] = points[i]-offset;
    }
    if(size == cap) expand();
}

LineBuffer::~LineBuffer(){
    delete[] vertices;
}

void Line::pushSegment(){
    LOG("Pushing segment");
    Point delta = (points[2]-points[0])/6.0f;
    Point c1 = points[1]+delta;
    delta = (points[3]-points[1])/6.0f;
    Point c2 = points[2]-delta;
    int i = 0;
    for(float t = 0.0, _t; t<1.01f; t+=INC){
        _t = (1.0f-t);
        tempPoints[i++] = _t*_t*_t*points[1]+3.0f*_t*_t*t*c1+3.0f*_t*t*t*c2+t*t*t*points[2];
    }
    startNorm = { points[0].y-points[2].y, points[2].x-points[0].x };
    float len = (float)sqrt(startNorm.x*startNorm.x+startNorm.y*startNorm.y);
    startNorm = startNorm/len;
    buffer.push(tempPoints, startNorm);
    drawSize += NUM_POINTS<<1;
}

Line::Line(Point start, uint width, uint height): size(1), drawSize(0){
    LOG("Creating Line");
    points = new Point[4];
    tempPoints = new Point[NUM_POINTS+1];
    points[0] = start;
    buffer.width = width;
    buffer.height = height;
}

void Line::push(Point p){
    LOG("Pushing point %f, %f", p.x, p.y);
    points[size++] = p;
    if(size >= 3){
        points[size-2] = (points[size-1]+points[size-2]+points[size-3])/3.0f;
        if(size == 4){
            pushSegment();
            for(uint i=0; i<3; ++i) points[i] = points[i+1];
            --size;
        }
    }
}

void Line::draw(){
    uint num = NUM_POINTS<<3;
    if(num > drawSize) num = drawSize;
    uint offset = drawSize-num;
    glBufferSubData(GL_ARRAY_BUFFER, 0, num*sizeof(Point), buffer.vertices+offset);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num);
}

Line::~Line(){
    delete[] points;
    delete[] tempPoints;
}