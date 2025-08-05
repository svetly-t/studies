#include "kid.h"

void KidInitialize(Kid &kid) {
    kid.state = Kid::STAND;
    kid.pos.x = 100;
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

struct Weights {
    double w0;
    double w1;
};

void KidUpdate(Kid &kid, KidUpdateContext ctx) {
    V2d intended_pos;
    V2d current_pos;
    V2d acc;
    LineToLineIntersection velocity_isct;
    LineToLineIntersection ground_isct;
    double run_speed;

    KeyState &ks = *(ctx.ks);
    Level &level = *(ctx.level);
    double dt = ctx.dt;

    Weights weights[kSwingPoints - 1];
    weights[0] = { 0.0, 1.0 };
    for (int i = 1; i < kSwingPoints - 1; ++i) {
        double factor = 0.5 - ((0.5 - 0.5) * (double)i / (kSwingPoints - 2));
        weights[i] = { factor, factor };
    }
    weights[kSwingPoints - 2] = { 0.99, 0.01 };

    switch (kid.state) {
        case Kid::STAND:
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            if (ks.x != 0) {
                KidSwitchState(kid, Kid::RUN);
                break;
            }
            if (ks.s > 0) {
                KidSwitchState(kid, Kid::CHARGE_JUMP);
                break;
            }
            if (!ground_isct.exists) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            break;
        case Kid::RUN:
            // Reset the state timer when directionality changes
            if (kid.vel.x * ks.x < 0) {
                kid.state_timer = 0.0;
            }
            // Change speed
            if (kid.state_timer < 0.33) {
                kid.vel.x = 20.0 * ks.x;
            } else {
                kid.vel.x = 100.0 * ks.x;
            }
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            kid.pos.x += kid.vel.x * dt;
            if (ks.x == 0) {
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            if (ks.s > 0) {
                KidSwitchState(kid, Kid::CHARGE_JUMP);
                break;
            }
            if (!ground_isct.exists) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            kid.state_timer += dt;
            break;
        case Kid::CHARGE_JUMP:
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            kid.pos.x += kid.vel.x * dt;
            kid.vel.x *= (1.0 - dt);
            kid.charge_timer += dt;
            if (ks.s == 0) {
                kid.vel.y = -50.0 * kid.charge_timer;
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            if (!ground_isct.exists) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            break;
        case Kid::JUMP:
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            if (velocity_isct.exists) {
                if (velocity_isct.normal.y < 0.0) {
                    kid.vel.y = 0.0;
                    KidSwitchState(kid, Kid::RUN);
                    break;
                }
            }
            kid.vel.y += 80.0 * dt;
            kid.pos += kid.vel * dt;
            kid.prev_pos = kid.pos;
            if (ks.s > 0) {
                kid.charge_timer += dt;
            } else if (kid.charge_timer > 0.0) {
                kid.swing_pos.x = kid.pos.x + (double)ks.x * 50.0 * kid.charge_timer;
                kid.swing_pos.y = kid.pos.y + (double)ks.y * 50.0 * kid.charge_timer;
                kid.swing_dist = (kid.swing_pos - kid.pos).Magnitude();
                KidSwitchState(kid, Kid::SWING);
                break;
            }
            break;
        case Kid::SWING:
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            intended_pos = kid.swing_pos + (kid.pos - kid.swing_pos).Normalized() * kid.swing_dist;
            acc = (intended_pos - kid.prev_pos);
            acc.y += 80.0;
            kid.pos = intended_pos;
            current_pos = kid.pos;
            kid.pos = current_pos * 2.0 - kid.prev_pos + acc * dt * dt;
            kid.vel = (current_pos - kid.prev_pos) / dt;
            kid.prev_pos = current_pos;
            if (ks.s > 0) {
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
