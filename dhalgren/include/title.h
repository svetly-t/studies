#pragma once

#include "v2d.h"

struct Title {
    enum State {
        HIDDEN,
        NOTHING,
        SELECTED
    };
    State state;
    double state_timer;

    V2d pos;
    V2d vel;
    V2d acc;
};

void TitleInitialize(Title &title);

void TitleUpdate(Title &title, KeyState &ks, double dt);