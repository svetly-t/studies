#pragma once

struct Title {
    enum State {
        NOTHING,
        SELECTED
    };
    State state;
    double state_timer;
};

void TitleInitialize(Title &title);

void TitleUpdate(Title &title, KeyState &ks, double dt);