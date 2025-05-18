#pragma once

#include "level.h"
#include "sdl_state.h"
#include "v2d.h"

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

    enum CollisionState {
        INSIDE,
        OUTSIDE
    };

    V2d swing_pos;
    double swing_dist;

    V2d pos;
    V2d vel;
};

void KidUpdate(Kid &kid, Level &level, KeyState &ks, double dt);