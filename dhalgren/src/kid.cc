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
    V2d ip;
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
            kid.pos += kid.vel * dt;
            kid.vel.y += 80.0 * dt;
            if (ks.s > 0) {
                kid.charge_timer += dt;
            } else if (kid.charge_timer > 0.0) {
                kid.swing_pos[0].x = kid.pos.x + (double)ks.x * 50.0 * kid.charge_timer;
                kid.swing_pos[0].y = kid.pos.y + (double)ks.y * 50.0 * kid.charge_timer;
                kid.swing_vel[0] = { 0.0, 0.0 };
                kid.swing_dist = (kid.swing_pos[0] - kid.pos).Magnitude();
                kid.swing_segment_dist = kid.swing_dist / (kSwingPoints - 1);
                for (int i = 1; i < kSwingPoints - 1; ++i) {
                    kid.swing_pos[i] = kid.swing_pos[0] + (kid.pos - kid.swing_pos[0]).Normalized() * i * kid.swing_segment_dist;
                    kid.swing_vel[i] = {0.0, 0.0};
                }
                kid.swing_pos[kSwingPoints - 1] = kid.pos;
                kid.swing_vel[kSwingPoints - 1] = kid.vel;
                KidSwitchState(kid, Kid::SWING);
                break;
            }
            break;
        case Kid::SWING:
            kid.swing_segment_dist_stretched = kid.swing_segment_dist;
            if (ks.e > 0) {
                kid.swing_segment_dist_stretched = kid.swing_segment_dist * 0.75;
            }
            for (int j = 0; j < kSwingPoints - 1; ++j) {
                V2d p0 = kid.swing_pos[j];
                V2d p1 = kid.swing_pos[j + 1];
                V2d vector = p1 - p0;
                V2d normal = vector.Normalized();
                double displacement = vector.Magnitude();
                double diff = kid.swing_segment_dist_stretched - displacement;
                V2d p0_1 = p0 - normal * (diff * weights[j].w0);
                V2d p1_1 = p1 + normal * (diff * weights[j].w1);
                kid.swing_pos[j] = p0_1;
                kid.swing_vel[j] += (p0_1 - p0) / dt;
                kid.swing_pos[j + 1] = p1_1;
                kid.swing_vel[j + 1] += (p1_1 - p1) / dt;
            }
            for (int j = 1; j < kSwingPoints; ++j) {
                kid.swing_vel[j].y += 80.0 * dt;
                kid.swing_pos[j] += kid.swing_vel[j] * dt;
            }
            kid.vel = (kid.swing_pos[kSwingPoints - 1] - kid.pos) / dt;
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            kid.pos += kid.vel * dt;
            kid.swing_pos[kSwingPoints - 1] = kid.pos;
            kid.swing_vel[kSwingPoints - 1] = kid.vel;
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
