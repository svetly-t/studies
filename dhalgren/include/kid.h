#pragma once

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

    V2d pos;
};

void KidUpdate(Kid &kid, KeyState &ks, float dt);