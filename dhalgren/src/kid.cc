#include "kid.h"

void KidSwitchState(Kid &kid, Kid::State new_state) {
    kid.state = new_state;
    kid.state_timer = 0.0;
    kid.charge_timer = 0.0;
}

LineToLineIntersection KidCollision(Kid &kid, KidUpdateContext ctx) {
    V2d pos_to_isct;
    LineToLineIntersection isct;

    Level &level = *(ctx.level);
    double dt = ctx.dt;
    double meters_per_pixel = ctx.meters_per_pixel;
    
    for (auto &aabb: level.aabbs) {
        isct = AABBToLineIntersect(aabb, kid.pos, kid.pos + kid.vel * dt);
        if (isct.exists) {
            pos_to_isct = isct.intersection_point - kid.pos;
            kid.pos = isct.intersection_point - pos_to_isct.Normalized() * meters_per_pixel;
            kid.vel = (isct.projection_point - isct.intersection_point) / dt;
            break;
        }
    }

    return isct;
}

void KidUpdate(Kid &kid, KidUpdateContext ctx) {
    V2d ip;
    LineToLineIntersection isct;

    KeyState &ks = *(ctx.ks);
    Level &level = *(ctx.level);
    double dt = ctx.dt;

    switch (kid.state) {
        case Kid::STAND:
            if (ks.x != 0) {
                KidSwitchState(kid, Kid::START_RUN);
                break;
            }
            break;
        case Kid::START_RUN:
            kid.state_timer += dt;
            if (kid.state_timer > 0.05) {
                kid.vel.x = ks.x * 50.0;
                KidSwitchState(kid, Kid::RUN);
                break;
            }
            if (ks.x == 0) {
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            break;
        case Kid::RUN:
            KidCollision(kid, ctx);
            kid.pos.x += kid.vel.x * dt;
            if (ks.x == 0) {
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            if (ks.x * kid.vel.x < 0) {
                KidSwitchState(kid, Kid::START_RUN);
                break;
            }
            if (ks.s > 0) {
                KidSwitchState(kid, Kid::CHARGE_JUMP);
                break;
            }
            break;
        case Kid::CHARGE_JUMP:
            KidCollision(kid, ctx);
            kid.pos.x += kid.vel.x * dt;
            kid.vel.x *= (1.0 - dt);
            kid.charge_timer += dt;
            if (ks.s == 0) {
                kid.vel.y = -50.0 * kid.charge_timer;
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            break;
        case Kid::JUMP:
            isct = KidCollision(kid, ctx);
            if (isct.exists) {
                if (isct.normal.y < 0.0) {
                    kid.vel.y = 0.0;
                    KidSwitchState(kid, Kid::RUN);
                    break;
                }
            }
            kid.pos.x += kid.vel.x * dt;
            kid.pos.y += kid.vel.y * dt;
            kid.vel.y += 10.0 * dt;
            if (kid.pos.y > 400.0) {
                kid.vel.y = 0.0;
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            if (ks.sp != 0.0) {
                KidSwitchState(kid, Kid::CHARGE_SHOT);
                break;
            }
            break;
        case Kid::CHARGE_SHOT:
            KidCollision(kid, ctx);
            kid.pos.x += kid.vel.x * dt;
            kid.pos.y += kid.vel.y * dt;
            kid.vel.y += 10.0 * dt;
            if (kid.pos.y > 400.0) {
                kid.vel.y = 0.0;
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            if (ks.s > 0.0) {
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
            KidCollision(kid, ctx);
            ip = kid.swing_pos + (kid.pos - kid.swing_pos).Normalized() * kid.swing_dist;
            kid.vel += (ip - kid.pos) / dt;
            kid.vel.y += 10.0 * dt;
            kid.pos = ip;
            kid.pos += kid.vel * dt;
            if (ks.sp != 0) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            break;
        default:
            break;
    }
}
