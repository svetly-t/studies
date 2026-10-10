#pragma once

#include "v2d.h"

struct Dude {
    enum Kind {
        RUNNING,
        STANDING
    };
    enum State {
        SAD,
        HAPPY
    };
    Kind kind;
    State state;
    V2d pos;
};

struct DudeUpdateContext {
    Level *level;
    RopeState *rs;
    double meters_per_pixel;
    double dt;
};

void DudeUpdate(Dude &dude, DudeUpdateContext ctx);