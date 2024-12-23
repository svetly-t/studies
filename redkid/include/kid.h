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

    enum State {
        WALKING,
        SLIDING,
        STOP_SLIDING,
        FALLING,
        SKIDDING,
        IDLE,
    };
    State state;
    V2d pos;
    V2d vel;
    V2d acc;
};
