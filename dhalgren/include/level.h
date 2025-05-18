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

void AABBToPointCollision(V2d final, V2d initial);

bool AABBToLineIntersect(AABB &aabb, Line& line, V2d i1, V2d i2);

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

void LevelUpdate(Level &level, KeyState &ks, double dt);