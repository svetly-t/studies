#include <cmath>
#include <cstdlib>

#include "kid.h"
#include "utilities.h"

void KidInitialize(Kid &kid) {
    kid.state = Kid::STAND;
    kid.pos.x = 0;
    kid.pos.y = 0;
}

void KidSwitchState(Kid &kid, Kid::State new_state) {
    kid.state = new_state;
    kid.state_timer = 0.0;
    kid.charge_timer = 0.0;
    kid.charge_started = false;
}

void KidCollision(
    KidUpdateContext ctx,
    V2d &pos,
    V2d &vel,
    LineToLineIntersection &velocity_isct,
    LineToLineIntersection &ground_isct
) {
    V2d pos_to_isct;
    V2d downward;

    Level &level = *(ctx.level);
    double dt = ctx.dt;
    double meters_per_pixel = ctx.meters_per_pixel;

    downward.y = 8.0;
    velocity_isct.exists = false;
    ground_isct.exists = false;
    
    for (auto &aabb: level.aabbs) {
        velocity_isct = AABBToLineIntersect(aabb, pos, pos + vel * dt);
        if (velocity_isct.exists) {
            // Ignore this collision if the line's normal doesn't point up,
            // or if we're approaching from underneath the line (i.e. dot(normal, vel) is g.t. zero)
            if (!(velocity_isct.normal.y < 0.0 && velocity_isct.normal * vel < 0.0))
                continue;
            pos_to_isct = velocity_isct.intersection_point - pos;
            pos = velocity_isct.intersection_point - pos_to_isct.Normalized() * meters_per_pixel;
            vel = (velocity_isct.projection_point - velocity_isct.intersection_point) / dt;
            break;
        }
        if (!ground_isct.exists)
            ground_isct = AABBToLineIntersect(aabb, pos, pos + downward);
    }
}

bool KidOverlap(KidUpdateContext ctx, V2d pos) {
    Level &level = *(ctx.level);
    double dt = ctx.dt;
    double meters_per_pixel = ctx.meters_per_pixel;
    
    for (auto &aabb: level.aabbs) {
        if (AABBToPointOverlap(aabb, pos)) {
            return true;
        }
    }
    
    return false;
}

V2d KidRopeFindAnchor(Kid &kid, KidUpdateContext ctx) {
    V2d anchor;
    
    KeyState &ks = *(ctx.ks);
    // RopeState &rs = *(ctx.rs);
    Level &level = *(ctx.level);

    kid.swing_reticle.width = kid.swing_reticle.height = 200.0;
    kid.swing_reticle.pos = kid.pos + V2d(ks.x, ks.y).Normalized() * 100.0 * (kid.charge_timer + 1.0) - V2d(kid.swing_reticle.width / 2.0, kid.swing_reticle.height / 2.0);
    
    // for (int i = 0; i < kRopePoints; ++i) {
    //     if (!rs.rope_points[i].active)
    //         continue;

    //     if (AABBToPointOverlap(kid.swing_reticle, rs.rope_points[i].pos))
    //         return rs.rope_points[i].pos;
    // }

    for (auto &aabb: level.aabbs)
        if (AABBToAABBOverlap(aabb, kid.swing_reticle, anchor))
            return anchor;

    return kid.pos + V2d(ks.x, ks.y).Normalized() * 100.0 * (kid.charge_timer + 1.0);
}

void KidRopeStart(Kid &kid, KidUpdateContext ctx, V2d endpoint) {
    kid.swing_pos[0] = endpoint;
    kid.swing_dist = (endpoint - kid.pos).Magnitude();
    for (int i = 0; i < kSwingPoints; ++i) {
        kid.swing_pos[i] = endpoint + (kid.pos - endpoint) * (double)i/(double)(kSwingPoints - 1);
        kid.swing_pos_prev[i] = endpoint + (kid.prev_pos - endpoint) * (double)i/(double)(kSwingPoints - 1);
    }
}

void KidRopeUpdate(Kid &kid, KidUpdateContext ctx) {
    KeyState &ks = *(ctx.ks);
    RopeState &rs = *(ctx.rs);

    rs.kid_gravity = 80.0 + abs(kid.vel.Normalized().Cross(V2d(0, 1))) * 160.0;
    rs.kid_acc = V2d(ks.x, ks.y).Normalized() * 20.0;
    kid.vel = rs.kid_vel;
    kid.prev_pos = rs.rope_points[kRopePoints].pos_prev;
    kid.pos = rs.rope_points[kRopePoints].pos;
}

// void KidRopeUpdate(Kid &kid, KidUpdateContext ctx, bool kid_is_fixed) {
//     V2d current_pos;
//     V2d acc;

//     double w1, w2;
//     double gravity;
//     double offset;
//     double real_swing_dist;
//     double swing_dist_offset;
//     double dt = ctx.dt;
//     double constraint_dist = kid.swing_dist / (double)(kSwingPoints - 1);
//     double last_constraint_dist = constraint_dist;
//     int point_count = kSwingPoints;

//     V2d effective_vel;
//     V2d line_to_center;

//     KeyState &ks = *(ctx.ks);

//     // Constrain all the rope points
//     for (int i = 1; i < kSwingPoints; ++i) {
//         if (i == 1) {
//             w1 = 0.0;
//             w2 = 1.0;
//         } else if (i == kSwingPoints - 1) {
//             w1 = 0.975;
//             w2 = 0.025;
//         } else {
//             w1 = 0.5;
//             w2 = 0.5;
//         }
//         offset = singleRopeConstraint(kid.swing_pos[i - 1], kid.swing_pos[i], w1, w2, constraint_dist);
//     }

//     real_swing_dist = (kid.swing_pos[kSwingPoints - 1] - kid.swing_pos[0]).Magnitude();

//     swing_dist_offset = real_swing_dist - kid.swing_dist;

//     // If the kid is fixed, don't do verlet integration on the last point in the array
//     // if (kid_is_fixed)
//     //     point_count = kSwingPoints - 1;

//     if (swing_dist_offset > 0.0)
//         gravity = 80.0 + abs(kid.vel.Normalized().Cross(V2d(0, 1))) * 160.0;
//     else
//         gravity = 160.0;

//     // Do verlet integration using the previous frame's rope points and this frame's
//     // See https://www.algorithm-archive.org/contents/verlet_integration/verlet_integration.html
//     for (int i = 1; i < point_count; ++i) {
//         acc = (kid.swing_pos[i] - kid.swing_pos_prev[i]);
//         if (i != kSwingPoints - 1)
//             acc.y += 80.0;
//         else {
//             acc.y += gravity;
//             if (swing_dist_offset > 0.0)
//                 acc += V2d(ks.x, ks.y).Normalized() * 10.0;
//         }
//         current_pos = kid.swing_pos[i];
//         // This is the verlet part: x(t + dt) = 2x        - x(x - dt) + a * dt**2
//         //                    i.e.: next_pos  = 2*cur_pos - prev_pos  + acceleration * dt**2
//         kid.swing_pos[i] = current_pos * 2.0 - kid.swing_pos_prev[i] + acc * dt * dt;
//         // Sometimes we have dt is 0.0; in that case the kid's vel goes to infinity 
//         if (dt > 0.0)
//             kid.vel = (current_pos - kid.swing_pos_prev[i]) / dt;
//         kid.swing_pos_prev[i] = current_pos;
//         kid.prev_pos = current_pos;
//     }

//     // kid.swing_pos[kSwingPoints - 1] = kid.swing_pos[kSwingPoints - 2];

//     // kid.swing_pos[kSwingPoints - 2] = kid.swing_pos[kSwingPoints - 1] + V2d(ks.x, ks.y).Normalized() * last_constraint_dist;

//     kid.pos = kid.swing_pos[kSwingPoints - 1];
// }

void KidStarUpdate(Kid &kid, KidUpdateContext ctx, double constraint_weight, bool drag) {
    const double kStarDist = 4.0;
    double dt = ctx.dt;
    double cos_angle = cos(kid.angle * 2 * 3.14159 / 360.0);
    double sin_angle = sin(kid.angle * 2 * 3.14159 / 360.0);
    V2d upwards = kid.visual_pos + V2d(sin_angle, -cos_angle) * kStarDist;
    V2d downwards = kid.visual_pos - V2d(sin_angle, -cos_angle) * kStarDist;
    V2d leftwards = kid.visual_pos - V2d(cos_angle, sin_angle) * kStarDist;
    V2d rightwards = kid.visual_pos + V2d(cos_angle, sin_angle) * kStarDist;
    V2d kid_vel_perp = V2d(-kid.vel.y, kid.vel.x);

    if (drag) {
        for (int i = 0; i < 3; ++i) {
            singleRopeConstraint(kid.star_pos[0], upwards, constraint_weight, 0.0, 0.0);
            singleRopeConstraint(kid.star_pos[1], downwards, constraint_weight, 0.0, 0.0);
            singleRopeConstraint(kid.star_pos[2], leftwards, constraint_weight, 0.0, 0.0);
            singleRopeConstraint(kid.star_pos[3], rightwards, constraint_weight, 0.0, 0.0);
        }
        
        for (int i = 0; i < 4; ++i) {
            double random_number = 0.0;
            double random_direction = random_number - (RAND_MAX / 2.0) > 0 ? 1.0 : -1.0;
            double random_scale = random_number / (double)RAND_MAX;
            kid.star_pos[i] += -(kid.vel + (kid_vel_perp * random_scale)) * dt;
        }
    } else {
        kid.star_pos[0] = upwards;
        kid.star_pos[1] = downwards;
        kid.star_pos[2] = leftwards;
        kid.star_pos[3] = rightwards;
    }
}

void KidVisualUpdate(Kid &kid, KidUpdateContext ctx, bool bob) {
    const double kBobDist = 2.0;
    const double kStarDist = 4.0;
    double bob_period_factor = 3.0 * (1.0 - abs(kid.vel.x) / 280.0);
    // double bob_period_increase_factor = log(abs(kid.vel.x) + 1.0);
    V2d downwards = V2d(0, kStarDist);
    V2d upwards = V2d(0, -kStarDist);

    KeyState &ks = *(ctx.ks);

    if (bob) {
        kid.visual_pos = kid.pos + upwards + downwards * (abs(sin(kid.state_timer * 2 * 3.14159 / (bob_period_factor))));
    } else {
        kid.visual_pos = kid.pos + upwards;
    }
}

struct Weights {
    double w0;
    double w1;
};

void KidUpdate(Kid &kid, KidUpdateContext ctx) {
    LineToLineIntersection velocity_isct;
    LineToLineIntersection ground_isct;
    double run_speed;

    KeyState &ks = *(ctx.ks);
    KeyState &ks_prev = *(ctx.ks_prev);
    RopeState &rs = *(ctx.rs);
    Level &level = *(ctx.level);
    double dt = ctx.dt;

    double sigmoid;
    double sigmoid_derivative;

    bool drag;

    const double kFallSpeed = 160.0;
    const double kDragFactor = 0.00001;

    if (ctx.ks->rp != 0) {
        KidInitialize(kid);
    }

    kid.angle += 800.0 * (kid.state_timer + 1.0) * ks.x * dt;

    switch (kid.state) {
        case Kid::STAND:
            kid.vel.x = 0;
            kid.vel.y = 0;
            KidCollision(ctx, kid.pos, kid.vel, velocity_isct, ground_isct);
            KidVisualUpdate(kid, ctx, false);
            KidStarUpdate(kid, ctx, 1.0, false);
            if (ks.x != 0) {
                KidSwitchState(kid, Kid::CHARGE_RUN);
                break;
            }
            if (!ground_isct.exists) {
                kid.vel.y += 80.0;
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            break;
        case Kid::CHARGE_RUN:
            // kid.pos.x += kid.vel.x * dt;
            KidStarUpdate(kid, ctx, 1.1, false);
            if (kid.charge_timer > 0.5) {
                kid.speed = 100.0;
                KidSwitchState(kid, Kid::RUN);
                break;
            }
            if (kid.state_timer > 0.1 && ks.x == 0) {
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            if (ks.x != 0) {
                kid.charge_timer += dt;
            }
            kid.state_timer += dt;
            break;
        case Kid::RUN:
            if (abs(kid.vel.x) < abs(kid.speed) && kid.speed > 100.0) {
                kid.speed = std::max(abs(kid.vel.x), 100.0);
            }
            // Run speed is on a predefined curve because I was chasing determinism
            // sigmoid:
            kid.vel.x = kid.speed * signOf(kid.charge_timer) / (1.0 + 100.0 * exp(-12.0 * abs(kid.charge_timer))) + 20.0 * signOf(kid.charge_timer);
            // logarithmic:
            //     kid.vel.x = 20.0 * ks.x * log(32.0 * kid.state_timer + 1.0);
            //     kid.angle += kid.vel.x * 16.0 * dt;
            KidCollision(ctx, kid.pos, kid.vel, velocity_isct, ground_isct);
            kid.pos.x += kid.vel.x * dt;
            KidStarUpdate(kid, ctx, 1.1, false);
            KidVisualUpdate(kid, ctx, false);
            if (ks.spcp == 1) {
                KidSwitchState(kid, Kid::JUMP);
                kid.vel.y = -40.0;
                kid.vel.x += signOf(kid.vel.x) * 20.0;
                break;
            }
            if (!ground_isct.exists) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            if (ks.x != 0) {
                kid.charge_timer += dt * ks.x;
            } else if (ks.x == 0) {
                if (abs(kid.vel.x) < 21.0) {
                    KidSwitchState(kid, Kid::STAND);
                    break;
                }
                kid.charge_timer -= signOf(kid.charge_timer) * dt;
            }
            kid.charge_timer = SDL_clamp(kid.charge_timer, -1.0, 1.0);
            kid.state_timer += dt;
            break;
        case Kid::JUMP:
            // Reduce gravity on the way up if holding spacebar
            // if (ks.spc > 0) {
            //     // Only resolve the collision if spacebar is not pressed. If it is, then pass through
            //     if (KidOverlap(ctx, kid.pos)) {
            //         kid.vel.x += ks.x * 10.0 * dt;
            //         if (ks.x * kid.vel.x > 0)
            //             kid.vel.y -= 320.0 * dt;
            //         else if (ks.x * kid.vel.x < 0)
            //             kid.vel.y += 320.0 * dt;
            //     } else {
            //         kid.vel.y += 80.0 * dt;
            //     }
            // } else {
            //     kid.vel.y += 160.0 * dt;
            //     KidCollision(ctx, kid.pos, kid.vel, velocity_isct, ground_isct);
            // }
            kid.vel.y += kFallSpeed * dt;
            kid.speed = kid.vel.y;
            KidCollision(ctx, kid.pos, kid.vel, velocity_isct, ground_isct);
            if (velocity_isct.exists) {
                if (velocity_isct.normal.y < 0.0) {
                    if (abs(kid.speed) < 50.0) {
                        KidSwitchState(kid, Kid::RUN);
                        if (abs(kid.vel.x) > 140.0) {
                            kid.charge_timer = 1.0 * signOf(kid.vel.x);
                            kid.speed = 100.0; // kid.vel.x - 40.0 * signOf(kid.vel.x);
                        } else {
                            kid.charge_timer = 0.25 * signOf(ks.x);
                            kid.speed = 100.0;
                        }
                        break;
                    } else {
                        kid.vel.y = -40.0;
                    }
                }
            }
            kid.prev_pos = kid.pos;
            kid.pos += kid.vel * dt;
            if (ks.s == 0 && ks.spcp > 0) {
                kid.charge_started = true;
                kid.charge_timer += dt;
            } else if (kid.charge_started && ks.spc > 0) {
                kid.charge_timer += dt;
                kid.swing_anchor = KidRopeFindAnchor(kid, ctx);
            } else if (kid.charge_started) {
                RopeAdd(rs, KidRopeFindAnchor(kid, ctx), kid.pos, kRopeLength, true, kid.prev_pos);
                // KidRopeStart(kid, ctx, KidRopeFindAnchor(kid, ctx)); // kid.pos + V2d(ks.x, ks.y).Normalized() * 100.0 * (kid.charge_timer + 1.0));
                KidSwitchState(kid, Kid::SWING);
                break;
            }
            if (ks.y == 1 && ks.spcp > 0) {
                if (KidOverlap(ctx, kid.pos)) {
                    kid.speed = kid.vel.Magnitude();
                    KidSwitchState(kid, Kid::CHARGE_BOUNCE);
                    break;
                }
            }
            kid.vel.x += -signOf(kid.vel.x) * (kid.vel.x * kid.vel.x) * kDragFactor;
            if (ks.x)
                drag = false;
            if (ks.spc == 0 && ks.x * kid.vel.x < 0) {
                kid.vel.x += ks.x * 75.0 * dt;
                kid.vel.y += kFallSpeed / 2.0 * dt;
                kid.charge_timer += dt;
            }
            if (ks.spc == 0 && ks.x * kid.vel.x > 0) {
                kid.vel.x += ks.x * 5.0 * dt;
            }
            if (ks.y == 1) {
                drag = true;
                kid.vel.y += kFallSpeed * dt;
            }
            if (kid.vel.y > 300.0)
                kid.vel.y = 300.0;
            KidStarUpdate(kid, ctx, 0.4, drag);
            KidVisualUpdate(kid, ctx, false);
            break;
        case Kid::CHARGE_BOUNCE:
            if (kid.state_timer >= 1.0 && kid.state == Kid::CHARGE_BOUNCE) {
                kid.speed += 75.0;
                kid.vel = V2d(ks.x, ks.y).Normalized() * kid.speed;
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            KidVisualUpdate(kid, ctx, false);
            KidStarUpdate(kid, ctx, 0.2, false);
            kid.state_timer += dt;
            // Alternative spin pattern for when we're not holding left or right
            if (ks.x == 0)
                kid.angle += (400.0 * signOf(ks.y) + 400.0 * ks.y) * (kid.state_timer + 1.0) * dt;
            break;
        case Kid::SWING:
            KidVisualUpdate(kid, ctx, false);
            KidRopeUpdate(kid, ctx);
            KidStarUpdate(kid, ctx, 0.2, false);
            if (ks.y != 1 && ks_prev.spcp == 1) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            if (ks.x || ks.y)
                kid.charge_timer += dt;
            else
                kid.charge_timer = 0;
            break;
        default:
            break;
    }
}
