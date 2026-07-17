#include "timer.h"

void TimerInitialize(Timer &timer) {
    timer.seconds = 180.0;
}

void TimerUpdate(Timer &timer, double dt) {
    timer.seconds -= dt;
}