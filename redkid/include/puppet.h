#pragma once

// class Puppet {
//  public:
//     virtual void Draw() = 0;
//     ~Puppet() {}
// };

class Leg {
 public:
    enum State {
        STANDING,
        SWINGING
    };
    State state;

    double kThetaSwingingKeyframes[4] = { 180.0, 90.0, 105.0, 180.0 };
    double kThetaStandingKeyframes[4] = { 180.0, 160.0, 170.0, 180.0 };

    double theta_offset;
    double theta_1;
    double theta_2;
    double timer;
    double period;
    double length;

    Leg(State state_);

    struct UpdateContext {
        double dt;
    };
    void Update(UpdateContext *ctx);
    void Draw();
};

class Body {
 public:
    void Update();
    void Draw();
};