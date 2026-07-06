#include "rope.h"
#include "utilities.h"

double singleRopeConstraint(V2d &pos1, V2d &pos2, double w1, double w2, double dist) {
    V2d real = pos2 - pos1;

    V2d dir = real.Normalized();

    double real_dist = real.Magnitude();

    double offset = real_dist - dist;

    // move pos1 towards pos2 by weight 1
    pos1 += dir * offset * w1;

    pos2 -= dir * offset * w2;

    return offset;
}

// prev_neighbor_idx points to the previous point in the rope array
// it's done this way to allow new rope points to reference ones that already exist,
// e.g. ones that are part of a laundry line
// The convention is that kRopePoints is the point that holds the player
//              therefore kRopePoints + kRopeLength is the last RopePoint in the array
// Returns the index of the last rope point that was created in this request
// Returns -1 if the requested rope cannot be created (e.g. no more space in the array)
int RopeCreate(RopeState &rs, V2d p2, V2d p1, int num_points, bool holding_player, V2d holding_player_pos_prev) {
    V2d pos;
    double dist;
    double total_dist;
    int last_point_idx;

    int rope_point_idx = rs.rope_point_idx;
    RopePoint *rope_points = rs.rope_points;

    if (holding_player) {
        rope_point_idx = kRopePoints;
    } else if (rope_point_idx + num_points >= kRopePoints) {
        return -1;
    }

    total_dist = (p2 - p1).Magnitude();

    dist = total_dist / (num_points - 1);

    last_point_idx = num_points - 1;

    for (int i = 0; i <= last_point_idx; ++i) {
        pos = p1 + (p2 - p1).Normalized() * dist * i;
        rope_points[rope_point_idx + i].pos = pos;
        rope_points[rope_point_idx + i].pos_prev = pos;
        rope_points[rope_point_idx + i].prev_neighbor_idx = -1;
        rope_points[rope_point_idx + i].prev_neighbor_dist = dist;
        rope_points[rope_point_idx + i].next_neighbor_idx = -1;
        rope_points[rope_point_idx + i].next_neighbor_dist = dist;
        rope_points[rope_point_idx + i].active = true;
        rope_points[rope_point_idx + i].fixed = false;
        rope_points[rope_point_idx + i].caught = false;
        rope_points[rope_point_idx + i].holding_player = false;
        if (i > 0) {
            rope_points[rope_point_idx + i].prev_neighbor_idx = rope_point_idx + i - 1;
        }
    }

    if (!holding_player) {
        rope_points[rope_point_idx].fixed = true;
        rope_points[rope_point_idx + last_point_idx].fixed = true;
    }

    if (holding_player) {
        rope_points[rope_point_idx].holding_player = true;
        rope_points[rope_point_idx + last_point_idx].fixed = true;
        rope_points[rope_point_idx].pos_prev = holding_player_pos_prev;
    }

    if (!holding_player)
        rs.rope_point_idx += num_points;

    return rope_point_idx + last_point_idx;
}

// Link a new rope to an existing RopePoint
int RopeCreateAndLink(RopeState &rs, RopePoint &p2, V2d p1, int num_points, bool holding_player, V2d holding_player_pos_prev) {
    int new_rope_tip_idx;

    new_rope_tip_idx = RopeCreate(rs, p2.pos, p1, num_points, holding_player, holding_player_pos_prev);

    if (new_rope_tip_idx == -1)
        return -1;

    // Link the old neighbor to the new one here...
    rs.rope_points[new_rope_tip_idx].next_neighbor_idx = p2.prev_neighbor_idx + 1;
    rs.rope_points[new_rope_tip_idx].fixed = false;

    return new_rope_tip_idx;
}

void RopeStateInitialize(RopeState &rs) {
    RopePoint *rope_points = rs.rope_points;

    rs.rope_point_idx = 0;

    for (int i = 0; i < kRopePointsTotal; ++i) {
        rope_points[i].fixed = false;
        rope_points[i].caught = false;
        rope_points[i].active = false;
        rope_points[i].holding_player = false;
        rope_points[i].prev_neighbor_idx = -1;
        rope_points[i].next_neighbor_idx = -1;
    }
}

void RopeStateUpdate(RopeState &rs, double dt) {
    V2d current_pos;
    V2d acc;
    V2d dir;

    V2d root_to_point;

    int i, j, k;
    int prev_neighbor_idx, prev_neighbor_neighbor_idx;
    int next_neighbor_idx;
    double w1, w2;
    double gravity;
    double real_swing_dist;
    double swing_dist_offset;
    double prev_neighbor_dist;
    double next_neighbor_dist;

    int &rope_point_idx = rs.rope_point_idx;
    RopePoint *rope_points = rs.rope_points;

    // Constrain all the rope points
    for (i = 0; i < kRopePointsTotal; ++i) {
        if (!rope_points[i].active)
            continue;

        if (rope_points[i].prev_neighbor_idx == -1)
            continue;

        root_to_point = rope_points[kRopePoints].pos - rope_points[i].pos;

        prev_neighbor_idx = rope_points[i].prev_neighbor_idx;
        prev_neighbor_dist = rope_points[i].prev_neighbor_dist;

        // This block only applies to the kid's ropePoints
        if (i >= kRopePoints) {
            if (rs.kid_stretch_state == RopeState::AFTER_STRETCH) {
                // prev_neighbor_dist = rope_points[i].prev_neighbor_dist - rs.kid_stretch_timer * 16.0;
                // if (prev_neighbor_dist < 0.1)
                //     prev_neighbor_dist = 0.1;
                prev_neighbor_dist = rope_points[i].prev_neighbor_dist * 0.5;
            }
        }

        next_neighbor_idx = rope_points[i].next_neighbor_idx;
        next_neighbor_dist = rope_points[i].next_neighbor_dist;

        if (rope_points[prev_neighbor_idx].fixed || rope_points[prev_neighbor_idx].caught) {
            w1 = 0.0;
            w2 = 1.0;
        } else if (rope_points[prev_neighbor_idx].holding_player) {
            switch (rs.kid_stretch_state) {
                // case RopeState::STRETCHED:
                //     w1 = 0.025 - rs.kid_stretch_timer;
                //     if (w1 < 0.01) {
                //         w1 = 0.01;
                //     }
                //     w2 = 1.0 - w1;
                //     break;
                case RopeState::UNROLLING:
                    w1 = 0.1;
                    w2 = 0.9;
                default:
                    w1 = 0.025;
                    w2 = 0.975;
            }
        } else {
            w1 = 0.5;
            w2 = 0.5;
        }
        if (rope_points[i].fixed || rope_points[i].caught) {
            w2 = 0.0;
        }
        singleRopeConstraint(rope_points[prev_neighbor_idx].pos, rope_points[i].pos, w1, w2, prev_neighbor_dist);

        // This is only evaluated if the kid rope is connected to another RopePoint
        // Otherwise next_neighbor_idx is generally -1
        if (next_neighbor_idx != -1) {
            w1 = 0.8;
            w2 = 0.2;
            singleRopeConstraint(rope_points[i].pos, rope_points[next_neighbor_idx].pos, w1, w2, next_neighbor_dist);
        }

        singleRopeConstraint(rope_points[prev_neighbor_idx].pos, rope_points[i].pos, w1, w2, prev_neighbor_dist);
    }

    gravity = 80.0;

    // Do verlet integration using the previous frame's rope points and this frame's
    // See https://www.algorithm-archive.org/contents/verlet_integration/verlet_integration.html
    for (i = 0; i < kRopePointsTotal; ++i) {
        if (!rope_points[i].active)
            continue;

        if (rope_points[i].fixed || rope_points[i].caught)
            continue;

        acc = (rope_points[i].pos - rope_points[i].pos_prev);

        if (rope_points[i].holding_player) {
            if (root_to_point.Magnitude() > rope_points[i].prev_neighbor_dist * kRopeLength) {
                // if (rs.kid_stretch) {
                //     rope_points[i].pos =
                //         rope_points[i + 1 /* should be next_neighbor_idx */].pos +
                //         root_to_point.Normalized() * (rope_points[i].prev_neighbor_dist + rs.kid_stretch_timer);
                // }
                acc.y += rs.kid_gravity;
                acc += rs.kid_acc;
            } else {
                // Makes the kid artificially heavier to give the swing more heft
                acc.y += 2.0 * gravity;
            }
            if (rs.kid_stretch_state == RopeState::STRETCHED) {
                acc += root_to_point.Normalized() * 80.0;
            }
        } else {
            acc.y += gravity;
        }

        rope_points[i].pos_current = rope_points[i].pos;
        // This is the verlet part: x(t + dt) = 2x        - x(x - dt) + a * dt**2
        //                    i.e.: next_pos  = 2*cur_pos - prev_pos  + acceleration * dt**2
        rope_points[i].pos = rope_points[i].pos_current * 2.0 - rope_points[i].pos_prev + acc * dt * dt;
        // Sometimes we have dt is 0.0; in that case the kid's vel goes to infinity
        if (dt > 0.0) {
            if (rope_points[i].holding_player) {
                rs.kid_vel = (rope_points[i].pos_current - rope_points[i].pos_prev) / dt;
            }
        }
    }

    const int kVertices = 4;
    V2d vertices[kVertices];

    for (i = kRopePoints; i < kRopePointsTotal - 1; ++i) {
        j = i + 1;

        if (!rope_points[i].active)
            continue;

        if (rope_points[i].fixed || rope_points[i].caught)
            continue;

        if (rope_points[j].fixed || rope_points[j].caught)
            continue;

        vertices[0] = rope_points[i].pos_prev;
        vertices[1] = rope_points[i].pos;
        vertices[2] = rope_points[j].pos;
        vertices[3] = rope_points[j].pos_prev;
        // loop over all collidables here
        for (k = 0; k < rs.collidable_aabb_count; ++k) {
            if (!PointInPolygon(rs.collidable_aabbs[k].pos, vertices, kVertices))
                continue;
            
            // If i is the player's index (i.e. kRopePoint), then the next point in the chain catches.
            // Else favor i, which should be lower in idx, and closer to the player.
            if (i == kRopePoints) {
                rope_points[j].caught = true;
                rope_points[j].pos_current = rope_points[j].pos = rs.collidable_aabbs[k].pos;
            } else {
                rope_points[i].caught = true;
                rope_points[i].pos_current = rope_points[i].pos = rs.collidable_aabbs[k].pos;
            }
        }
    }

    for (i = 0; i < kRopePointsTotal; ++i) {
        if (!rope_points[i].active)
            continue;

        if (rope_points[i].fixed || rope_points[i].caught)
            continue;

        rope_points[i].pos_prev = rope_points[i].pos_current;
    }
}
