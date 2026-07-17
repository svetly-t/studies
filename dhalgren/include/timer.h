#pragma once

struct Timer {
    double seconds;
};

void TimerInitialize(Timer &timer);

void TimerUpdate(Timer& timer, double dt);
