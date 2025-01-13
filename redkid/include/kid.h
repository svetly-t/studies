#pragma once

#include "v2d.h"
#include "sdl_state.h"
#include "terrain.h"

class Kid {
 public:
    Kid() { state = State::SLIDING; }

    void Initialize(double x, double y);
    struct UpdateContext {
        KeyState *ks;
        Terrain *terrainp;
        double dt;
        double gravity;
    };
    void Update(UpdateContext *ctx);

    struct FlyingContext {
        double angle;
        double wind_sign;
        double min_cl;
        double max_cl;
        double max_aoa;
        double max_aoa_cosine;
        double stall_angle;
        double stall_angle_cosine;
    };
    FlyingContext flying_ctx;

    struct StateContext {
        double timer;
        int x;
    };
    StateContext state_ctx;

    enum State {
        AUTO_WALKING,
        WALKING,
        SLIDING,
        STUCK,
        FLYING,
        FALLING,
        STALLING,
        IDLE,
    };
    State state;
    V2d pos;
    V2d vel;
    V2d acc;
};
