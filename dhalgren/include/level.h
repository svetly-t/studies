#pragma once

#include "v2d.h"
#include "sdl_state.h"

#include <unordered_map>
#include <vector>

struct Line {
    V2d p1;
    V2d p2;
};

struct AABB {
    V2d pos;
    double width;
    double height;
};

struct RopePoint {
    V2d pos;
    V2d pos_prev;
    int neighbor_idx;
    double neighbor_dist;
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
};

void RopeAdd(RopeState &rs, V2d p2, V2d p1, int num_points, bool holding_player, V2d holding_player_pos_prev);

void RopeAdd(RopeState &rs, RopePoint &p2, V2d p1, int num_points, bool holding_player);

void RopeStateInitialize(RopeState &rope_state);

void RopeStateUpdate(RopeState &rope_state, double dt);

bool AABBToPointOverlap(AABB &aabb, V2d final);

bool AABBToAABBOverlap(AABB target, AABB sweep, V2d &overlap);

struct LineToLineIntersection {
    bool exists;
    V2d l1;
    V2d l2;
    V2d intersection_point;
    V2d projection_point;
    V2d normal;
};

LineToLineIntersection AABBToLineIntersect(AABB &aabb, V2d i1, V2d i2);

LineToLineIntersection AABBToLineIntersect(AABB &aabb, Line l);

struct Chunk {
    static const int kWidth = 800;
    static const int kHeight = 600;
    AABB aabb;
};

struct Level {
    enum State {
        READY_BOX,
        READY_LINE,
        ADJUST_BOX,
        ADJUST_LINE,
        RANDOM_POPULATE_START,
        RANDOM_POPULATE_DONE,
    };
    State state;

    V2d l1, l2;

    int window_x;
    int window_y;

    AABB aabb;
    std::vector<AABB> aabbs;
    std::unordered_map<uint64_t, Chunk> chunk_map;
};

uint64_t LevelChunkMapIndex(double x, double y);

void LevelChunkMapUpdate(AABB aabb, int aabb_index);

void LevelInitialize(Level &level, int window_x, int window_y);

void LevelUpdate(Level &level, RopeState& rs, KeyState &ks, V2d mouse_pos, double dt);
