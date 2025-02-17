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
        int *sprite_frame;
        int *sprite_flip;
        double dt;
        double gravity;
    };
    void Update(UpdateContext *ctx);

    struct StateContext {
        double timer;
        double frame_timer;
        int last_held_x;
    };
    StateContext state_ctx;

    enum State {
        AUTO_WALKING,
        WALKING,
        SLIDING,
        BECOME_STUCK,
        STUCK,
        FALLING,
        BECOME_IDLE,
        IDLE,
        BECOME_HIGHEST_PEAK,
        HIGHEST_PEAK,
    };
    State state;
    V2d pos;
    V2d vel;
    V2d acc;
};
