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
            w1 = 0.99;
            w2 = 0.01;
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

struct Weights {
    double w0;
    double w1;
};

void KidUpdate(Kid &kid, KidUpdateContext ctx) {
    LineToLineIntersection velocity_isct;
    LineToLineIntersection ground_isct;
    double run_speed;

    KeyState &ks = *(ctx.ks);
    Level &level = *(ctx.level);
    double dt = ctx.dt;

    switch (kid.state) {
        case Kid::STAND:
            KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            if (ks.x != 0) {
                KidSwitchState(kid, Kid::RUN);
                break;
            }
            if (ks.spc > 0) {
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
            if (ks.spc > 0) {
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
            if (ks.spc == 0) {
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
            kid.prev_pos = kid.pos;
            kid.pos += kid.vel * dt;
            if (ks.spc > 0) {
                kid.charge_timer += dt;
            } else if (kid.charge_timer > 0.0) {
                KidRopeStart(kid, ctx, kid.pos + V2d(ks.x, ks.y) * 50.0 * kid.charge_timer);
                KidSwitchState(kid, Kid::SWING);
                break;
            }
            break;
        case Kid::SWING:
            // KidCollision(kid.pos, kid.vel, velocity_isct, ground_isct, ctx);
            // intended_pos = kid.swing_pos + (kid.pos - kid.swing_pos).Normalized() * kid.swing_dist;
            // acc = (intended_pos - kid.prev_pos);
            // acc.y += 80.0;
            // kid.pos = intended_pos;
            // current_pos = kid.pos;
            // kid.pos = current_pos * 2.0 - kid.prev_pos + acc * dt * dt;
            // kid.vel = (current_pos - kid.prev_pos) / dt;
            // kid.prev_pos = current_pos;
            KidRopeUpdate(kid, ctx);
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
