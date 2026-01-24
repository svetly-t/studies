#include "level.h"
#include "sdl_state.h"
#include "utilities.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>

bool AABBToPointOverlap(AABB &aabb, V2d final) {
    if (final.y > aabb.pos.y &&
        final.x > aabb.pos.x &&
        final.y < aabb.pos.y + aabb.height &&
        final.x < aabb.pos.x + aabb.width)
            return true;
    return false;
}

// This isn't really AABB-to-AABB; if reticle is entirely within the target it fails
bool AABBToAABBOverlap(AABB target, AABB reticle, V2d &overlap) {
    if (AABBToPointOverlap(reticle, target.pos)) {
        overlap = target.pos;
        return true;
    }
    if (AABBToPointOverlap(reticle, target.pos + V2d(target.width, 0))) {
        overlap = target.pos + V2d(target.width, 0);
        return true;
    }
    if (AABBToPointOverlap(reticle, target.pos + V2d(target.width, target.height))) {
        overlap = target.pos + V2d(target.width, target.height);
        return true;
    }
    if (AABBToPointOverlap(reticle, target.pos + V2d(0, target.height))) {
        overlap = target.pos + V2d(0, target.height);
        return true;
    }
    return false;
}

V2d LineToLineProjectionPoint(V2d l1, V2d l2, V2d i1, V2d i2) {
    V2d tangent = (l2 - l1).Normalized();
    return l1 + tangent * ((i2 - l1) * tangent);
}

V2d LineToLineIntersectionPoint(V2d l1, V2d l2, V2d i1, V2d i2) {
    V2d normal = (l2 - l1).Orthogonal();
    double proj_i2 = std::abs((i2 - l1) * normal);
    double proj_i1 = std::abs((i1 - l1) * normal);
    double ratio = proj_i1 / (proj_i1 + proj_i2);

    return i1 + ((i2 - i1) * ratio);
}

V2d LineToLineIntersectionNormal(V2d l1, V2d l2, V2d i1, V2d i2) {
    V2d normal = (l2 - l1).Orthogonal();
    return normal.Normalized(); // * std::copysign(1.0, (l2-l1).Cross(i1-l1)) * -1.0 /* I don't know why this -1.0 is needed */;
}

LineToLineIntersection LineToLineIntersect(V2d l1, V2d l2, V2d i1, V2d i2) {
    double a1;
    double a2;
    double a3;
    double a4;

    LineToLineIntersection isct;

    isct.exists = false;
    isct.l1 = l1;
    isct.l2 = l2;

    a1 = (l2 - l1).Cross(i1 - l1);
    a2 = (l2 - l1).Cross(i2 - l1);
    if (std::copysign(1.0, a1) == std::copysign(1.0, a2))
        return isct;

    a3 = std::abs((i2 - i1).Cross(l1 - i1));
    a4 = std::abs((i2 - i1).Cross(l2 - i1));
    if (a4 > a3)
        a3 = a4;

    if (!(a3 < std::abs(a1) + std::abs(a2)))
        return isct;

    isct.exists = true;
    isct.intersection_point = LineToLineIntersectionPoint(l1, l2, i1, i2);
    isct.projection_point = LineToLineProjectionPoint(l1, l2, i1, i2);
    isct.normal = LineToLineIntersectionNormal(l1, l2, i1, i2);

    return isct;
}

LineToLineIntersection AABBToLineIntersect(AABB &aabb, V2d i1, V2d i2) {
    V2d aabb_width = { aabb.width, 0.0 };
    V2d aabb_height = { 0.0, aabb.height };

    LineToLineIntersection isct;

    isct = LineToLineIntersect(aabb.pos, aabb.pos + aabb_width, i1, i2);
    if (isct.exists)
        return isct;
    isct = LineToLineIntersect(aabb.pos + aabb_width, aabb.pos + aabb_width + aabb_height, i1, i2);
    if (isct.exists)
        return isct;
    isct = LineToLineIntersect(aabb.pos + aabb_width + aabb_height, aabb.pos + aabb_height, i1, i2);
    if (isct.exists)
        return isct;
    isct = LineToLineIntersect(aabb.pos + aabb_height, aabb.pos, i1, i2);
    if (isct.exists)
        return isct;

    return isct;
}

LineToLineIntersection AABBToLineIntersect(AABB &aabb, Line l) {
    return AABBToLineIntersect(aabb, l.p1, l.p2);
}

void RopeAdd(RopeState &rs, V2d p2, V2d p1, int num_points, bool holding_player, V2d holding_player_pos_prev) {
    V2d pos;
    double dist;
    double total_dist;
    int last_point_idx;

    int rope_point_idx = rs.rope_point_idx;
    RopePoint *rope_points = rs.rope_points;

    if (holding_player) {
        rope_point_idx = kRopePoints;
    } else if (rope_point_idx + num_points >= kRopePoints) {
        return;
    }

    total_dist = (p2 - p1).Magnitude();

    dist = total_dist / (num_points - 1);

    last_point_idx = num_points - 1;

    for (int i = 0; i <= last_point_idx; ++i) {
        pos = p1 + (p2 - p1).Normalized() * dist * i;
        rope_points[rope_point_idx + i].pos = pos;
        rope_points[rope_point_idx + i].pos_prev = pos;
        rope_points[rope_point_idx + i].neighbor_idx = -1;
        rope_points[rope_point_idx + i].neighbor_dist = dist;
        rope_points[rope_point_idx + i].active = true;
        rope_points[rope_point_idx + i].fixed = false;
        rope_points[rope_point_idx + i].holding_player = false;
        if (i > 0) {
            rope_points[rope_point_idx + i].neighbor_idx = rope_point_idx + i - 1;
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
}

void RopeAdd(RopeState &rs, RopePoint &p2, V2d p1, int num_points, bool holding_player) {
    return;
}

void RopeStateInitialize(RopeState &rs) {
    RopePoint *rope_points = rs.rope_points;

    rs.rope_point_idx = 0;

    for (int i = 0; i < kRopePointsTotal; ++i) {
        rope_points[i].fixed = false;
        rope_points[i].active = false;
        rope_points[i].holding_player = false;
        rope_points[i].neighbor_idx = -1;
    }
}

void RopeStateUpdate(RopeState &rs, double dt) {
    V2d current_pos;
    V2d acc;

    int neighbor_idx;
    double w1, w2;
    double gravity;
    double real_swing_dist;
    double swing_dist_offset;
    double neighbor_dist;

    int &rope_point_idx = rs.rope_point_idx;
    RopePoint *rope_points = rs.rope_points;

    // Constrain all the rope points
    for (int i = 0; i < kRopePointsTotal; ++i) {
        if (!rope_points[i].active)
            continue;

        if (rope_points[i].neighbor_idx == -1)
            continue;
        
        neighbor_idx = rope_points[i].neighbor_idx;
        neighbor_dist = rope_points[i].neighbor_dist;

        if (rope_points[neighbor_idx].fixed) {
            w1 = 0.0;
            w2 = 1.0;
        } else if (rope_points[neighbor_idx].holding_player) {
            w1 = 0.025;
            w2 = 0.975;
        } else {
            w1 = 0.5;
            w2 = 0.5;
        }
        if (rope_points[i].fixed) {
            w2 = 0.0;
        }
        singleRopeConstraint(rope_points[neighbor_idx].pos, rope_points[i].pos, w1, w2, neighbor_dist);
    }

    gravity = 80.0;

    // Do verlet integration using the previous frame's rope points and this frame's
    // See https://www.algorithm-archive.org/contents/verlet_integration/verlet_integration.html
    for (int i = 0; i < kRopePointsTotal; ++i) {
        if (!rope_points[i].active)
            continue;

        if (rope_points[i].fixed)
            continue;

        acc = (rope_points[i].pos - rope_points[i].pos_prev);

        if (rope_points[i].holding_player) {
            acc.y += rs.kid_gravity;
            acc += rs.kid_acc;
        } else {
            acc.y += gravity;
        }

        current_pos = rope_points[i].pos;
        // This is the verlet part: x(t + dt) = 2x        - x(x - dt) + a * dt**2
        //                    i.e.: next_pos  = 2*cur_pos - prev_pos  + acceleration * dt**2
        rope_points[i].pos = current_pos * 2.0 - rope_points[i].pos_prev + acc * dt * dt;
        // Sometimes we have dt is 0.0; in that case the kid's vel goes to infinity 
        if (dt > 0.0) {
            if (rope_points[i].holding_player) {
                rs.kid_vel = (current_pos - rope_points[i].pos_prev) / dt;
            }
        }
        rope_points[i].pos_prev = current_pos;
    }
}

void LevelInitialize(Level &level, int window_x, int window_y) {
    level.state = Level::RANDOM_POPULATE_START;
    level.window_x = window_x;
    level.window_y = window_y;
}

void LevelSwitchState(Level &level, Level::State new_state) {
    level.state = new_state;
}

void LevelRandomPopulate(Level &level, RopeState &rs) {
    double x, y, width, height;

    level.aabbs.clear();
    level.aabb.height = 0;
    level.aabb.width = 0;
    
    // Always place a box directly beneath origin so that the player has something to fall on
    x = -50;
    y = 100;
    width = 100.0;
    height = 100.0;
    level.aabbs.push_back(AABB{
        .pos = { x, y },
        .width = width,
        .height = height
    });

    for (int i = 1; i < 10; ++i) {
        AABB test;
        V2d test_overlap;
        bool overlap = false;

        x = rand() % level.window_x - level.window_x / 2;
        y = rand() % level.window_y - level.window_y / 2;
        width = rand() % 100 + 100;
        height = rand() % 100 + 100;

        test = AABB{
            .pos = { x, y },
            .width = width,
            .height = height
        };

        // Make sure that none of the corners overlap with any of the other boxes
        // This doesn't totally prevent overlap
        for (auto &aabb : level.aabbs) {
            if (AABBToAABBOverlap(aabb, test, test_overlap) ||
                AABBToAABBOverlap(test, aabb, test_overlap)) {
                overlap = true;
                break;
            }
        }

        if (overlap) {
            --i;
            continue;
        }

        level.aabbs.push_back(AABB{
            .pos = { x, y },
            .width = width,
            .height = height
        });

        if (rand() % 8 == 0 && i > 1) {
            RopeAdd(rs, level.aabbs[i].pos, level.aabbs[i - 1].pos, 10, false, {0, 0});
        }
    }
}

void LevelSave(Level &level) {
    int written;
    char buf[256];
    const char *filename = "data.lvl";
    SDL_RWops *file = SDL_RWFromFile(filename, "r+");

    if (!file) {
        printf("%s does not exist, attempting to create...\n", filename);

        file = SDL_RWFromFile(filename, "w+");

        if (!file) {
            printf("cannot open %s, aborting...\n", filename);
            exit(1);
        }
    }

    for (AABB &aabb : level.aabbs) {
        written = snprintf(buf, sizeof(buf), "%lf,%lf,%lf,%lf\n", aabb.pos.x, aabb.pos.y, aabb.width, aabb.height);
        SDL_RWwrite(file, buf, written, 1);
    }

    SDL_RWclose(file);
}

void LevelLoad(Level &level) {
    AABB aabb;
    char buf[256];
    size_t current_offset = 0;
    size_t seek = 0;
    const char *filename = "data.lvl";
    SDL_RWops *file = SDL_RWFromFile(filename, "r");

    if (!file) {
        printf("%s does not exist, returning...\n", filename);
        return;
    }

    level.aabbs.clear();

    while (1) {
        current_offset = SDL_RWtell(file);
        seek = 0;

        if (SDL_RWread(file, buf, 1, sizeof(buf)) == 0) {
            printf("Error reading file: %s\n", SDL_GetError());
            break;
        }

        while (buf[seek++] != '\n') {}

        SDL_RWseek(file, current_offset + seek, RW_SEEK_SET);
        
        sscanf(buf, "%lf,%lf,%lf,%lf", &aabb.pos.x, &aabb.pos.y, &aabb.width, &aabb.height);
        
        level.aabbs.push_back(AABB{
            .pos = { aabb.pos.x, aabb.pos.y },
            .width = aabb.width,
            .height = aabb.height
        });
    }
}

bool LevelRemoveBox(Level &level, V2d mouse_pos) {
    for (size_t i = 0; i < level.aabbs.size(); ++i) {
        if (AABBToPointOverlap(level.aabbs[i], mouse_pos)) {
            level.aabbs[i] = level.aabbs[level.aabbs.size() - 1];
            level.aabbs.pop_back();
            return true;
        }
    }
    return false;
}

void LevelUpdate(Level &level, RopeState &rs, KeyState &ks, V2d &mouse_pos, double dt) {
    bool removed_box = false;
    switch (level.state) {
        case Level::READY_BOX:
            // if (ks.ep != 0) {
            //     LevelSwitchState(level, Level::READY_LINE);
            //     break;
            // }
            if (ks.mrcp != 0) {
                if (LevelRemoveBox(level, mouse_pos))
                    break;
            }
            if (ks.mlc != 0) {
                LevelSwitchState(level, Level::ADJUST_BOX);
                level.aabb.pos = mouse_pos;
                break;
            }
            if (ks.rp != 0) {
                LevelSwitchState(level, Level::RANDOM_POPULATE_START);
                break;
            }
            if (ks.sp != 0) {
                LevelSave(level);
            }
            if (ks.lp != 0) {
                LevelLoad(level);
            }
            break;
        case Level::RANDOM_POPULATE_START:
            RopeStateInitialize(rs);
            LevelRandomPopulate(level, rs);
            LevelSwitchState(level, Level::RANDOM_POPULATE_DONE);
            break;
        case Level::RANDOM_POPULATE_DONE:
            if (ks.rp == 0) {
                LevelSwitchState(level, Level::READY_BOX);
                break;
            }
            break;
        case Level::READY_LINE:
            if (ks.ep != 0) {
                LevelSwitchState(level, Level::READY_BOX);
                break;
            }
            if (ks.mlc != 0) {
                LevelSwitchState(level, Level::ADJUST_LINE);
                level.l1 = mouse_pos;
                break;
            }
            if (ks.sp != 0) {
                LevelSave(level);
            }
            if (ks.lp != 0) {
                LevelLoad(level);
            }
            break;
        case Level::ADJUST_LINE:
            level.l2 = mouse_pos;
            if (ks.mlc == 0) {
                LevelSwitchState(level, Level::READY_LINE);
                break;
            }
            break;
        case Level::ADJUST_BOX:
            level.aabb.width = mouse_pos.x - level.aabb.pos.x;
            level.aabb.height = mouse_pos.y - level.aabb.pos.y;
            if (ks.mlc == 0) {
                if (level.aabb.width < 0.0) {
                    level.aabb.pos.x += level.aabb.width;
                    level.aabb.width = -level.aabb.width;
                }
                if (level.aabb.height < 0.0) {
                    level.aabb.pos.y += level.aabb.height;
                    level.aabb.height = -level.aabb.height;
                }
                level.aabbs.push_back(level.aabb);
                LevelSwitchState(level, Level::READY_BOX);
                break;
            }
            break;
    }
}