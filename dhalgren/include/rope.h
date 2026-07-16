#pragma once

#include "v2d.h"
#include "collisions.h"

struct RopeParameters {
    bool is_pole;
    bool is_holding_player;
    V2d pos_prev_player;
    V2d base;
    V2d tip;

    int num_points;
};

struct RopePoint {
    V2d pos;
    V2d pos_prev;
    V2d pos_current;

    int prev_neighbor_idx;
    double prev_neighbor_dist;

    int next_neighbor_idx;
    double next_neighbor_dist;

    bool caught;
    bool fixed;
    bool holding_player;
    bool active;

    bool pole;
    bool pole_tip;
};

const int kRopeLength = 10;
const int kRopePoints = kRopeLength * 10;
const int kRopePointsTotal = 128;

struct RopeState {
    int rope_point_idx;
    RopePoint rope_points[kRopePointsTotal];

    int collidable_aabb_count;
    AABB *collidable_aabbs;

    // These get passed from the KidUpdate to the RopeUpdate
    // depending on length of rope + KeyState
    double kid_gravity;
    V2d kid_acc;
    // This is passed from the RopeState back to the kid;
    // it is the mid-integration velocity of the point holding the kid
    V2d kid_vel;

    enum State {
        UNSTRETCHED,
        STRETCHED,
        UNROLLING,
        AFTER_STRETCH
    };
    State kid_stretch_state;
    double kid_stretch_timer;
};

int RopeCreate(RopeState &rs, RopeParameters p);

int RopeCreateAndLink(RopeState &rs, RopePoint &existing, RopeParameters p);

void RopeStateInitialize(RopeState &rope_state);

void RopeStateUpdate(RopeState &rope_state, double dt);
