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
        int last_held_x;
        double hoof_speed;
        double hoof_timer;
    };
    StateContext state_ctx;

    enum State {
        BECOME_WALKING,
        BECOME_RUNNING,
        AUTO_HOOFING,
        HOOFING,
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
