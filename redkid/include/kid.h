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
        V2d bearing;
        double min_cl;
        double max_cl;
        double max_aoa;
        double max_aoa_cosine;
        double stall_angle;
        double stall_angle_cosine;
    };
    FlyingContext flying_ctx;

    enum State {
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
