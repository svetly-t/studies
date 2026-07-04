#pragma once

#include "v2d.h"
#include "sdl_state.h"
#include "collisions.h"
#include "rope.h"

#include <unordered_map>
#include <vector>

struct Level {
    enum State {
        READY_BOX,
        READY_LINE,
        ADJUST_BOX,
        ADJUST_LINE,
        RANDOM_POPULATE_START,
        RANDOM_POPULATE_DONE,
    };
    State state;

    V2d l1, l2;

    int window_x;
    int window_y;

    AABB aabb;
    std::vector<AABB> aabbs;

    std::vector<Circle> circles;
};

void LevelInitialize(Level &level, int window_x, int window_y);

void LevelUpdate(Level &level, RopeState& rs, KeyState &ks, V2d mouse_pos, double dt);
