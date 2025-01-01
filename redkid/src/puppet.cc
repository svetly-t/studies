#include "puppet.h"
#include "lerp.h"

#include <cmath>

Leg::Leg(State state_) {
    state = state_;
    theta_offset = 60.0;
    theta_1 = theta_offset;
    theta_2 = 180.0;
    timer = 0;
    period = 16.0 * (1.0 / 24.0);
    length = 0.25;
}

void Leg::Update(UpdateContext *ctx) {
    double theta_span = 180.0 - 2.0 * theta_offset;

    switch (state) {
        case SWINGING:
            // theta_1 = LerpBetween(theta_offset, 180.0 - theta_offset, timer, period);
            theta_1 = theta_offset + theta_span * std::sin(timer / period * 3.14159 / 2.0);
            theta_2 = PiecewiseLerpBetween(kThetaSwingingKeyframes, 4, theta_1 - theta_offset, theta_span);
            break;
        case STANDING:
            // theta_1 = LerpBetween(180.0 - theta_offset, theta_offset, timer, period);
            theta_1 = 180.0 - theta_offset - theta_span * std::sin(timer / period * 3.14159 / 2.0);
            theta_2 = PiecewiseLerpBetween(kThetaStandingKeyframes, 4, theta_span - (theta_1 - theta_offset), theta_span);
            break;
    }

    timer += ctx->dt;
    if (timer >= period) {
        switch (state) {
            case SWINGING:
                state = STANDING;
            break;
            case STANDING:
                state = SWINGING;
            break;
        }
        timer = 0;
    }
}