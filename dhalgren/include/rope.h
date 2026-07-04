#pragma once

#include "v2d.h"

struct RopePoint {
    V2d pos;
    V2d pos_prev;

    int prev_neighbor_idx;
    double prev_neighbor_dist;

    int next_neighbor_idx;
    double next_neighbor_dist;

    bool fixed;
    bool holding_player;
    bool active;
};

const int kRopeLength = 10;
const int kRopePoints = kRopeLength * 10;
const int kRopePointsTotal = 128;

struct RopeState {
    int rope_point_idx;
    RopePoint rope_points[kRopePointsTotal];

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

int RopeCreate(RopeState &rs, V2d p2, V2d p1, int num_points, bool holding_player, V2d holding_player_pos_prev);

int RopeCreateAndLink(RopeState &rs, RopePoint &p2, V2d p1, int num_points, bool holding_player, V2d holding_player_pos_prev);

void RopeStateInitialize(RopeState &rope_state);

void RopeStateUpdate(RopeState &rope_state, double dt);
