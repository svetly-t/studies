#pragma once

#include "v2d.h"
#include "sdl_state.h"

#include <vector>

struct Line {
    V2d p1;
    V2d p2;
};

struct AABB {
    V2d pos;
    double width;
    double height;
};

struct LineToLineIntersection {
    bool exists;
    V2d l1;
    V2d l2;
    V2d intersection_point;
    V2d projection_point;
    V2d normal;
};

LineToLineIntersection AABBToLineIntersect(AABB &aabb, V2d i1, V2d i2);

LineToLineIntersection AABBToLineIntersect(AABB &aabb, Line l);

struct Level {
    enum State {
        READY_BOX,
        READY_LINE,
        ADJUST_BOX,
        ADJUST_LINE,
    };
    State state;

    V2d l1, l2;

    AABB aabb;
    std::vector<AABB> aabbs;
};

void LevelInitialize(Level &level, int window_x, int window_y);

void LevelUpdate(Level &level, KeyState &ks, double dt);