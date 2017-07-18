#ifndef GL_NATIVE_LINE_H
#define GL_NATIVE_LINE_H

#include <GLES3/gl3.h>
#include <EGL/egl.h>

#define NUM_POINTS 10
#define INC 1.0f/NUM_POINTS

typedef unsigned int uint;

struct Point{
    GLfloat x;
    GLfloat y;
};

Point operator+(Point p1, Point p2){
    return (Point){p1.x+p2.x, p1.y+p2.y};
}

Point operator/(Point p, float val){
    return (Point){p.x/val, p.y/val};
}

Point operator*(float val, Point p){
    return (Point){val*p.x, val*p.y};
}

Point operator-(Point p1, Point p2){
    return (Point){p1.x-p2.x, p1.y-p2.y};
}

class LineBuffer{
    void expand();

public:
    uint width;
    uint height;
    Point *vertices;
    uint size;
    uint cap;

    LineBuffer();
    void push(Point *points, Point startNorm);
    ~LineBuffer();
};

class Line{
    LineBuffer buffer;

    Point *points;
    Point *tempPoints;
    Point startNorm;

    uint drawSize;

    void pushSegment();

public:

    uint size;

    Line(Point start, uint width, uint height);
    void push(Point p);
    void draw();
    ~Line();
};

#endif
