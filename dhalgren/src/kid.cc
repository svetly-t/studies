#include "kid.h"

void KidSwitchState(Kid &kid, Kid::State new_state) {
    kid.state = new_state;
    kid.state_timer = 0.0;
    kid.charge_timer = 0.0;
}

void KidCollision(
    Kid &kid,
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
    
    for (auto &aabb: level.aabbs) {
        velocity_isct = AABBToLineIntersect(aabb, kid.pos, kid.pos + kid.vel * dt);
        if (velocity_isct.exists) {
            pos_to_isct = velocity_isct.intersection_point - kid.pos;
            kid.pos = velocity_isct.intersection_point - pos_to_isct.Normalized() * meters_per_pixel;
            kid.vel = (velocity_isct.projection_point - velocity_isct.intersection_point) / dt;
            break;
        }
        if (!ground_isct.exists)
            ground_isct = AABBToLineIntersect(aabb, kid.pos, kid.pos + downward);
    }
}

void KidUpdate(Kid &kid, KidUpdateContext ctx) {
    V2d ip;
    LineToLineIntersection velocity_isct;
    LineToLineIntersection ground_isct;
    double run_speed;

    KeyState &ks = *(ctx.ks);
    Level &level = *(ctx.level);
    double dt = ctx.dt;

    switch (kid.state) {
        case Kid::STAND:
            KidCollision(kid, velocity_isct, ground_isct, ctx);
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
            KidCollision(kid, velocity_isct, ground_isct, ctx);
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
            KidCollision(kid, velocity_isct, ground_isct, ctx);
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
            KidCollision(kid, velocity_isct, ground_isct, ctx);
            if (velocity_isct.exists) {
                if (velocity_isct.normal.y < 0.0) {
                    kid.vel.y = 0.0;
                    KidSwitchState(kid, Kid::RUN);
                    break;
                }
            }
            kid.pos.x += kid.vel.x * dt;
            kid.pos.y += kid.vel.y * dt;
            kid.vel.y += 10.0 * dt;
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
            KidCollision(kid, velocity_isct, ground_isct, ctx);
            ip = kid.swing_pos + (kid.pos - kid.swing_pos).Normalized() * kid.swing_dist;
            kid.vel += (ip - kid.pos) / dt;
            kid.vel.y += 10.0 * dt;
            kid.pos = ip;
            kid.pos += kid.vel * dt;
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
