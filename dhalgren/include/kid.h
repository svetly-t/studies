#pragma once

#include "level.h"
#include "sdl_state.h"
#include "v2d.h"

const int kSwingPoints = 10;

struct Kid {
    enum State {
        STAND,
        START_RUN,
        RUN,
        CHARGE_JUMP,
        JUMP,
        CHARGE_SHOT,
        SHOT,
        SWING,
    };
    State state;
    double state_timer;
    double charge_timer;

    V2d swing_pos[kSwingPoints];
    V2d swing_pos_prev[kSwingPoints];
    double swing_dist;

    V2d star_pos[4];
    V2d star_pos_prev[4];

    V2d prev_pos;
    V2d pos;
    V2d vel;
};

struct KidUpdateContext {
    Level *level;
    KeyState *ks;
    double meters_per_pixel;
    double dt;
};

void KidInitialize(Kid &kid);

void KidUpdate(Kid &kid, KidUpdateContext ctx);
