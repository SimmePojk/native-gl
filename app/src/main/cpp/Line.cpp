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

void LineBuffer::push(Point point, Point norm){
    vertices[size++] = point+norm;
    vertices[size++] = point-norm;
    if(size == cap) expand();
}

LineBuffer::~LineBuffer(){
    delete[] vertices;
}

/*void Line::pushSegment(){
    LOG("Pushing segment");
    Point tempPoint, tempNorm;
    Point tangent_a = 0.5f*(points[2]-points[0]);
    Point tangent_b = 0.5f*(points[3]-points[1]);
    Point norm_a = { -tangent_a.y, tangent_a.x };
    float len = (float)sqrt(norm_a.x*norm_a.x+norm_a.y*norm_a.y);
    norm_a = (10.0f / len) * norm_a;
    norm_a.x /= width;
    norm_a.y /= height;
    Point norm_b = { -tangent_b.y, tangent_b.x };
    len = (float)sqrt(norm_b.x*norm_b.x+norm_b.y*norm_b.y);
    norm_b = (10.0f / len) * norm_b;
    norm_b.x /= width;
    norm_b.y /= height;
    float h1, h2, h3, h4;
    for(float t=0.0f, t2; t<1.0f; t+=INC){
        t2 = t*t;
        h1 = t2*(2*t-3)+1;
        h2 = t2*(3-2*t);
        h3 = t*(t*(t-2)+1);
        h4 = t2*(t-1);
        tempPoint = h1*points[1]+h2*points[2]+h3*tangent_a+h4*tangent_b;
        tempNorm = h1*norm_a+h2*norm_b;
        buffer.push(tempPoint, tempNorm);
    }
    drawSize+=NUM_POINTS<<1;
}*/

/*void Line::pushSegment() {
    LOG("Pushing segment");
    Point tempPoint, tempNorm;
    Point delta = (points[2]-points[0])/6.0f;
    Point c1 = points[1]+delta;
    delta = (points[3]-points[1])/6.0f;
    Point c2 = points[2]-delta;
    float len;
    for(float t = 0.0f, _t; t<1.0f; t+=INC){
        _t = 1-t;
        tempPoint = (_t*_t*_t)*points[1]+(3.0f*_t*_t*t)*c1+(3.0f*_t*t*t)*c2+(t*t*t)*points[2];
        tempNorm = (3.0f*_t*_t)*(c1-points[1])+(6.0f*_t*t)*(c2-c1)+(3.0f*t*t)*(points[2]-c2);
        tempNorm = { -tempNorm.y, tempNorm.x };
        len = (float)sqrt(tempNorm.x*tempNorm.x+tempNorm.y*tempNorm.y);
        tempNorm = (10.0f / len) * tempNorm;
        tempNorm.x /= width;
        tempNorm.y /= height;
        buffer.push(tempPoint, tempNorm);
    }
    drawSize+=NUM_POINTS<<1;
}*/

void Line::pushSegment(){
    LOG("Pushing segment");
    Point point;
    Point delta = (points[2]-points[0])/6.0f;
    Point norm = { -delta.y, delta.x };
    float len = (float)sqrt(norm.x*norm.x+norm.y*norm.y);
    norm = (3.0f / len) * norm;
    norm.x /= width;
    norm.y /= height;
    Point start_a = points[1]+norm, start_b = points[1]-norm;
    Point c1_a = start_a+delta, c1_b = start_b+delta;
    delta = (points[3]-points[1])/6.0f;
    norm = { -delta.y, delta.x };
    len = (float)sqrt(norm.x*norm.x+norm.y*norm.y);
    norm = (3.0f / len) * norm;
    norm.x /= width;
    norm.y /= height;
    Point end_a = points[2]+norm, end_b = points[2]-norm;
    Point c2_a = end_a-delta, c2_b = end_b-delta;
    int i=0;
    for(float t=0.0f, _t, a, b, c, d; t<1.0f; t+=INC){
        _t = 1-t;
        a = _t*_t*_t;
        b = 3.0f*_t*_t*t;
        c = 3.0f*_t*t*t;
        d = t*t*t;
        point = a*start_a+b*c1_a+c*c2_a+d*end_a;
        buffer.vertices[buffer.size++] = point;
        point = a*start_b+b*c1_b+c*c2_b+d*end_b;
        buffer.vertices[buffer.size++] = point;
        i+=2;
    }
    if(buffer.size == buffer.cap) buffer.expand();
    drawSize += NUM_POINTS<<1;
}


Line::Line(Point start, uint width, uint height): size(1), drawSize(0){
    LOG("Creating Line");
    points = new Point[4];
    points[0] = start;
    this->width = width;
    this->height = height;
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
    uint num = (NUM_POINTS+1)<<2;
    if(num > drawSize) num = drawSize;
    uint offset = drawSize-num;
    //glBufferData(GL_ARRAY_BUFFER, drawSize*sizeof(Point), vertices, GL_STATIC_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, num*sizeof(Point), buffer.vertices+offset);
    Point *data = (Point*)glMapBufferRange(GL_ARRAY_BUFFER, 0, num*sizeof(Point), GL_MAP_WRITE_BIT);
    for(uint i=0; i<num; ++i) data[i] = buffer.vertices[i+offset];
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num);
}

Line::~Line(){
    delete[] points;
}