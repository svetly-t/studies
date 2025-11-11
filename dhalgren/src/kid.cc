#include <cmath>
#include <cstdlib>

#include "kid.h"
#include "utilities.h"

void KidInitialize(Kid &kid) {
    kid.state = Kid::STAND;
    kid.pos.x = 100;
    kid.pos.y = 0;
}

void KidSwitchState(Kid &kid, Kid::State new_state) {
    kid.state = new_state;
    kid.state_timer = 0.0;
    kid.charge_timer = 0.0;
}

void KidCollision(
    V2d &pos,
    V2d &vel,
    LineToLineIntersection &velocity_isct,
    LineToLineIntersection &ground_isct,
    KidUpdateContext ctx
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
            pos_to_isct = velocity_isct.intersection_point - pos;
            pos = velocity_isct.intersection_point - pos_to_isct.Normalized() * meters_per_pixel;
            vel = (velocity_isct.projection_point - velocity_isct.intersection_point) / dt;
            break;
        }
        if (!ground_isct.exists)
            ground_isct = AABBToLineIntersect(aabb, pos, pos + downward);
    }
}

void KidRopeStart(Kid &kid, KidUpdateContext ctx, V2d endpoint) {
    kid.swing_pos[0] = endpoint;
    kid.swing_dist = (endpoint - kid.pos).Magnitude();
    for (int i = 0; i < kSwingPoints; ++i) {
        kid.swing_pos[i] = endpoint + (kid.pos - endpoint) * (double)i/(double)(kSwingPoints - 1);
        kid.swing_pos_prev[i] = endpoint + (kid.prev_pos - endpoint) * (double)i/(double)(kSwingPoints - 1);
    }
}

void singleRopeConstraint(V2d &pos1, V2d &pos2, double w1, double w2, double dist) {
    V2d real = pos2 - pos1;

    V2d dir = real.Normalized();

    double real_dist = real.Magnitude();

    double offset = real_dist - dist;

    // move pos1 towards pos2 by weight 1
    pos1 += dir * offset * w1;

    pos2 -= dir * offset * w2;
}

void KidRopeUpdate(Kid &kid, KidUpdateContext ctx) {
    V2d current_pos;
    V2d acc;

    double w1, w2;
    double dt = ctx.dt;
    double constraint_dist = kid.swing_dist / (double)(kSwingPoints - 1);

    // Constrain all the rope points
    for (int i = 1; i < kSwingPoints; ++i) {
        if (i == 1) {
            w1 = 0;
            w2 = 1.0;
        } else if (i == kSwingPoints - 1) {
            w1 = 0.95;
            w2 = 0.05;
        } else {
            w1 = 0.5;
            w2 = 0.5;
        }
        singleRopeConstraint(kid.swing_pos[i - 1], kid.swing_pos[i], w1, w2, constraint_dist);
    }

    // Do verlet integration using the previous frame's rope points and this frame's
    // See https://www.algorithm-archive.org/contents/verlet_integration/verlet_integration.html
    for (int i = 1; i < kSwingPoints; ++i) {
        acc = (kid.swing_pos[i] - kid.swing_pos_prev[i]);
        acc.y += 80.0;
        current_pos = kid.swing_pos[i];
        // This is the verlet part: x(t + dt) = 2x        - x(x - dt) + a * dt**2
        //                    i.e.: next_pos  = 2*cur_pos - prev_pos  + acceleration * dt**2
        kid.swing_pos[i] = current_pos * 2.0 - kid.swing_pos_prev[i] + acc * dt * dt;
        // Sometimes we have dt is 0.0; in that case the kid's vel goes to infinity 
        if (dt > 0.0)
            kid.vel = (current_pos - kid.swing_pos_prev[i]) / dt;
        kid.swing_pos_prev[i] = current_pos;
        kid.prev_pos = current_pos;
    }

    kid.pos = kid.swing_pos[kSwingPoints - 1];
}

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
    Level &level = *(ctx.level);
    double dt = ctx.dt;

    double sigmoid;
    double sigmoid_derivative;

    if (ctx.ks->rp != 0) {
        KidInitialize(kid);
    }

    kid.angle += 800.0 * (kid.state_timer + 1.0) * ks.x * dt;

    switch (kid.state) {
        case Kid::STAND:
            kid.vel.x = 0;
            kid.vel.y = 0;
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            KidVisualUpdate(kid, ctx, false);
            KidStarUpdate(kid, ctx, 1.0, false);
            if (ks.x != 0) {
                KidSwitchState(kid, Kid::CHARGE_RUN);
                break;
            }
            if (ks.spc > 0) {
                KidSwitchState(kid, Kid::CHARGE_JUMP);
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
                kid.speed = abs(kid.vel.x);
            }
            // Change speed
            // sigmoid:
            kid.vel.x = kid.speed * signOf(kid.charge_timer) / (1.0 + 100.0 * exp(-12.0 * abs(kid.charge_timer))) + 20.0 * signOf(kid.charge_timer);
            // logarithmic:
            // kid.vel.x = 20.0 * ks.x * log(32.0 * kid.state_timer + 1.0);
            // kid.angle += kid.vel.x * 16.0 * dt;
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            kid.pos.x += kid.vel.x * dt;
            KidStarUpdate(kid, ctx, 1.1, false);
            KidVisualUpdate(kid, ctx, false);
            if (ks.spcp == 1) {
                KidSwitchState(kid, Kid::JUMP);
                kid.vel.y = -40.0;
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
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            if (velocity_isct.exists) {
                if (velocity_isct.normal.y < 0.0) {
                    kid.vel.y = 0.0;
                    KidSwitchState(kid, Kid::RUN);
                    if (abs(kid.vel.x) > 140.0) {
                        kid.charge_timer = 1.0 * signOf(kid.vel.x);
                        kid.speed = kid.vel.x - 40.0 * signOf(kid.vel.x);
                    } else {
                        kid.charge_timer = 0.25 * signOf(ks.x);
                    }
                    break;
                }
            }
            if (ks.spc > 0 && kid.vel.y < 0) {
                kid.vel.y += 80.0 * dt;
            } else {
                kid.vel.y += 160.0 * dt;
            }
            kid.prev_pos = kid.pos;
            kid.pos += kid.vel * dt;
            KidStarUpdate(kid, ctx, 0.4, true);
            KidVisualUpdate(kid, ctx, false);
            if (ks.spcp > 0) {
                kid.charge_timer += dt;
            } else if (kid.charge_timer > 0.0 && ks.spc > 0) {
                kid.charge_timer += dt;
            } else if (kid.charge_timer > 0.0) {
                KidRopeStart(kid, ctx, kid.pos + V2d(ks.x, ks.y) * 50.0 * kid.charge_timer);
                KidSwitchState(kid, Kid::SWING);
                break;
            }
            break;
        case Kid::SWING:
            KidVisualUpdate(kid, ctx, false);
            KidRopeUpdate(kid, ctx);
            KidStarUpdate(kid, ctx, 0.2, true);
            if (ks.spc > 0) {
                kid.charge_timer += dt;
            } else if (kid.charge_timer > 0.0) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            break;
        default:
            break;
    }
}
