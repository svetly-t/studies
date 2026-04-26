#include <cmath>
#include <cstdlib>

#include "kid.h"
#include "utilities.h"

bool TouchEventNear(V2d final, V2d initial, double mag) {
    if ((final - initial).SqrMagnitude() < mag * mag)
        return true;
    return false;
}

int TouchEventToX(V2d final, V2d initial, double mag) {
    if (final.x - initial.x > mag)
        return 1;
    if (final.x - initial.x < -mag)
        return -1;
    return 0;
}

int TouchEventToY(V2d final, V2d initial, double mag) {
    if (final.y - initial.y > mag)
        return 1;
    if (final.y - initial.y < -mag)
        return -1;
    return 0;
}

void KidInitialize(Kid &kid) {
    kid.state = Kid::STAND;
    kid.pos.x = 0;
    kid.pos.y = 0;
    kid.using_touch = false;
}

void KidSwitchState(Kid &kid, Kid::State new_state) {
    kid.state = new_state;
    kid.state_timer = 0.0;
    kid.charge_timer = 0.0;
    kid.charge_started = false;
}

struct KidCollision {
    LineToLineIntersection velocity_isct;
    LineToLineIntersection ground_isct;
    bool circle_isct;
};

void KidCollisionCheck(
    Kid &kid,
    KidUpdateContext ctx,
    KidCollision &kid_collision
) {
    V2d pos_to_isct;
    V2d downward;

    Level &level = *(ctx.level);
    LineToLineIntersection &velocity_isct = kid_collision.velocity_isct;
    LineToLineIntersection &ground_isct = kid_collision.ground_isct;
    V2d &pos = kid.pos;
    V2d &vel = kid.vel;
    bool &circle_isct = kid_collision.circle_isct;

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

    for (auto &circle: level.circles) {
        if ((circle.pos - pos).SqrMagnitude() > circle.sqrRadius) {
            circle_isct = true;
            break;
        }
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

V2d KidRopeFindAnchor(Kid &kid, KidUpdateContext ctx, RopePoint *&rp) {
    V2d anchor;
    
    KeyState &ks = *(ctx.ks);
    RopeState &rs = *(ctx.rs);
    Level &level = *(ctx.level);
    V2d mouse_pos = ctx.mouse_pos;

    rp = nullptr;

    kid.swing_reticle.width = kid.swing_reticle.height = 200.0;

    if (ks.t) {
        kid.swing_reticle.pos = mouse_pos - V2d(kid.swing_reticle.width / 2.0, kid.swing_reticle.height / 2.0);
    } else {
        kid.swing_reticle.pos = kid.pos + V2d(ks.x, ks.y).Normalized() * 100.0 * (kid.charge_timer + 1.0) - V2d(kid.swing_reticle.width / 2.0, kid.swing_reticle.height / 2.0);
    }

    for (int i = 0; i < kRopePoints; ++i) {
        if (!rs.rope_points[i].active)
            continue;
        if (!rs.rope_points[i].pole_tip)
            continue;
        if (AABBToPointOverlap(kid.swing_reticle, rs.rope_points[i].pos)) {
            rp = &rs.rope_points[i];
            return rs.rope_points[i].pos;
        }
    }

    for (auto &aabb: level.aabbs)
        if (AABBToAABBOverlap(aabb, kid.swing_reticle, anchor))
            return anchor;

    return kid.swing_reticle.pos + V2d(kid.swing_reticle.width / 2.0, kid.swing_reticle.height / 2.0);
}

void KidRopeStart(Kid &kid, KidUpdateContext ctx, V2d endpoint) {
    kid.swing_pos[0] = endpoint;
    kid.swing_dist = (endpoint - kid.pos).Magnitude();
    for (int i = 0; i < kSwingPoints; ++i) {
        kid.swing_pos[i] = endpoint + (kid.pos - endpoint) * (double)i/(double)(kSwingPoints - 1);
        kid.swing_pos_prev[i] = endpoint + (kid.prev_pos - endpoint) * (double)i/(double)(kSwingPoints - 1);
    }
}

void KidRopeUpdate(Kid &kid, KidUpdateContext ctx, double acceleration_multiplier) {
    KeyState &ks = *(ctx.ks);
    RopeState &rs = *(ctx.rs);

    rs.kid_gravity = 80.0 + abs(kid.vel.Normalized().Cross(V2d(0, 1))) * 160.0;
    rs.kid_acc = V2d(ks.x, ks.y).Normalized() * 20.0 * acceleration_multiplier;
    kid.vel = rs.kid_vel;
    kid.prev_pos = rs.rope_points[kRopePoints].pos_prev;
    kid.pos = rs.rope_points[kRopePoints].pos;
}

void KidStarUpdate(Kid &kid, KidUpdateContext ctx, double constraint_weight) {
    const double kStarDist = 4.0;
    const double kPi = 3.14159;
    double angle;
    double angle_separator;
    double cos_angle;
    double sin_angle;
    double dt = ctx.dt;
    // Look at the desmos for the function sin(pi/2 * x)
    // It is similar to the curve you get when doing iterative frame-by-frame interpolation from 0 to 1.
    double fadein_clamp;
    
    if (kid.state_timer > 0.5)
        fadein_clamp = 1.0;
    else
        fadein_clamp = SDL_clamp(sin(kPi / 2.0 * kid.state_timer * 2.0), 0.0, 1.0);

    angle = kid.angle * 2 * kPi / 360.0;

    switch (kid.state) {
        // case Kid::UNSPLAT:
        //     angle_separator = kPi / 2.0 * fadein_clamp;
        //     cos_angle = cos(angle + angle_separator);
        //     sin_angle = sin(angle + angle_separator);
        //     kid.star_pos[0] = kid.visual_pos + V2d(cos(angle), sin(angle)) * kStarDist;
        //     kid.star_pos[1] = kid.visual_pos + V2d(cos(angle + 1.0 * angle_separator), sin(angle + 1.0 * angle_separator)) * kStarDist;
        //     kid.star_pos[2] = kid.visual_pos + V2d(cos(angle + 2.0 * angle_separator), sin(angle + 2.0 * angle_separator)) * kStarDist;
        //     kid.star_pos[3] = kid.visual_pos + V2d(cos(angle + 3.0 * angle_separator), sin(angle + 3.0 * angle_separator)) * kStarDist;
        //     break;
        default:
            cos_angle = cos(angle);
            sin_angle = sin(angle);
            kid.star_pos[0] = kid.visual_pos + V2d(sin_angle, -cos_angle) * kStarDist;
            kid.star_pos[1] = kid.visual_pos - V2d(sin_angle, -cos_angle) * kStarDist;
            kid.star_pos[2] = kid.visual_pos - V2d(cos_angle, sin_angle) * kStarDist;
            kid.star_pos[3] = kid.visual_pos + V2d(cos_angle, sin_angle) * kStarDist;
            break;
    }
}

void KidVisualUpdate(Kid &kid, KidUpdateContext ctx, bool bob) {
    RopeState &rs = *(ctx.rs);
    KeyState &ks = *(ctx.ks);

    const double kPi = 3.14159;
    const double kKidHeight = 8.0;
    V2d downwards = V2d(0, kKidHeight);
    V2d upwards = V2d(0, -kKidHeight);
    V2d rightwards = V2d(1.0, 0.0);

    // kUnsplatVZero tells us how quickly the star bounces up after the kid unsplats.
    // unsplat_gravity comes from solving the kinematics equation for gravity, when v_zero and t are fixed.
    const double kUnsplatVZero = 17.5;
    double unsplat_gravity = 2 * kUnsplatVZero / kid.kUnsplatSeconds;

    double visual_angle;

    switch (kid.state) {
        case Kid::SWING:
            kid.visual_pos = kid.pos; // + downwards;
            // visual_angle = acos(rightwards * (rs.rope_points[kRopePoints + 1].pos - rs.rope_points[kRopePoints].pos).Normalized());
            // kid.visual_angle = -(visual_angle * 180.0 / kPi - 90.0);
            break;
        case Kid::UNSPLAT:
            kid.visual_pos = kid.pos + upwards + upwards * (-unsplat_gravity / 2.0 * kid.state_timer * kid.state_timer + kUnsplatVZero * kid.state_timer);
            kid.visual_angle = 0;
            break;
        default:
            kid.visual_pos = kid.pos + upwards;
            kid.visual_angle = 0;
            break;
    }
}

const double kFallSpeed = 160.0;
const double kPi = 3.14159;

// Return true if Kid hits the ground, false otherwise
bool KidJumpUpdate(Kid &kid, KidUpdateContext ctx, KidCollision &kid_collision) {
    KeyState ks = *(ctx.ks);
    double dt = ctx.dt;

    kid.vel.y += kFallSpeed * dt;
    // We save the kid.vel.y into kid.speed here because the velocity gets
    // overwritten by the KidCollision func, and we need kid.vel.y to determine
    // if this collision is a SPLAT or not.
    kid.speed = kid.vel.y;
    KidCollisionCheck(kid, ctx, kid_collision);
    if (kid_collision.velocity_isct.exists) {
        if (kid_collision.velocity_isct.normal.y < 0.0) {
            if (abs(kid.speed) < 100.0) {
                KidSwitchState(kid, Kid::RUN);
                if (abs(kid.vel.x) > 70.0) {
                    kid.charge_timer = 1.0 * signOf(kid.vel.x);
                    kid.speed = abs(kid.vel.x); // - 40.0 * signOf(kid.vel.x);
                } else {
                    kid.charge_timer = 0.25 * signOf(ks.x);
                    kid.speed = 100.0;
                }
                return true;
            } else {
                kid.speed = kid.vel.x;
                kid.angle = 3.0 * kPi / 4.0;
                KidSwitchState(kid, Kid::SPLAT);
                return true;
            }
        }
    }
    kid.prev_pos = kid.pos;
    kid.pos += kid.vel * dt;

    return false;
}

void KidUpdate(Kid &kid, KidUpdateContext ctx) {
    KidCollision kid_collision;

    KeyState ks = *(ctx.ks);
    KeyState ks_prev = *(ctx.ks_prev);
    RopeState &rs = *(ctx.rs);
    Level &level = *(ctx.level);
    V2d mouse_pos = ctx.mouse_pos;
    double dt = ctx.dt;

    bool drag;

    const double kDragFactor = 0.00001;
    const double kParrySeconds = 0.25;

    double touch_charge_timer_multiplier = 1.0;
    double touch_swing_acceleration_multiplier = 1.0;
    bool touch_jump_event = false;
    bool touch_release_event = false;

    V2d ks_dir;
    V2d rs_dir;

    RopePoint *rp;

    if (ks.rp != 0) {
        KidInitialize(kid);
    }

    // toggle -- if kb starts being used,
    // ignore the touch stuff
    if (ks.t) {
        kid.using_touch = true;
        touch_charge_timer_multiplier = 2.0;
        touch_swing_acceleration_multiplier = 16.0;
    } else if (ks.x || ks.y || ks.spc) {
        kid.using_touch = false;
        touch_jump_event = false;
        touch_release_event = false;
        touch_charge_timer_multiplier = 1.0;
        touch_swing_acceleration_multiplier = 1.0;
    }

    if (kid.using_touch) {
        if (ks.t) {
            ks.x = TouchEventToX(mouse_pos, kid.pos, 80.0);
            ks.y = TouchEventToY(mouse_pos, kid.pos, 80.0);
        } else {
            if (ks_prev.t == 1)
                touch_release_event = true;
            ks.x = 0;
            ks.y = 0;
        }
        if (ks.tp) {
            touch_jump_event = TouchEventNear(mouse_pos, kid.pos, 80.0);
        }
    }

    kid.angle += 800.0 * (kid.state_timer + 1.0) * ks.x * dt;

    switch (kid.state) {
        case Kid::STAND:
            kid.vel.x = 0;
            kid.vel.y = 0;
            KidCollisionCheck(kid, ctx, kid_collision);
            KidVisualUpdate(kid, ctx, false);
            KidStarUpdate(kid, ctx, 1.0);
            if (ks.x != 0) {
                KidSwitchState(kid, Kid::CHARGE_RUN);
                break;
            }
            if (!kid_collision.ground_isct.exists) {
                kid.vel.y += 80.0;
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            kid.state_timer += dt;
            break;
        case Kid::CHARGE_RUN:
            // kid.pos.x += kid.vel.x * dt;
            KidStarUpdate(kid, ctx, 1.1);
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
            kid.vel.x = kid.speed * signOf(kid.charge_timer) /
                (1.0 + 100.0 * exp(-12.0 * abs(kid.charge_timer * touch_charge_timer_multiplier))) +
                 20.0 * signOf(kid.charge_timer * touch_charge_timer_multiplier);
            // logarithmic:
            //     kid.vel.x = 20.0 * ks.x * log(32.0 * kid.state_timer + 1.0);
            //     kid.angle += kid.vel.x * 16.0 * dt;
            KidCollisionCheck(kid, ctx, kid_collision);
            kid.pos.x += kid.vel.x * dt;
            KidStarUpdate(kid, ctx, 1.1);
            KidVisualUpdate(kid, ctx, false);
            if (ks.spcp || touch_jump_event) {
                KidSwitchState(kid, Kid::JUMP);
                kid.vel.y = -40.0;
                kid.vel.x += signOf(kid.vel.x) * 20.0;
                break;
            }
            if (!kid_collision.ground_isct.exists) {
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
        case Kid::SPLAT:
            kid.vel.x = SDL_clamp(1.0 - kid.state_timer, 0.0, 1.0) * kid.speed;
            kid.pos.x += kid.vel.x * dt;
            KidStarUpdate(kid, ctx, 1.1);
            KidVisualUpdate(kid, ctx, false);
            KidCollisionCheck(kid, ctx, kid_collision);
            if (!kid_collision.ground_isct.exists) {
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            kid.state_timer += dt;
            if (kid.state_timer > 1.0) {
                KidSwitchState(kid, Kid::UNSPLAT);
                break;
            }
            break;
        case Kid::UNSPLAT:
            KidStarUpdate(kid, ctx, 1.1);
            KidVisualUpdate(kid, ctx, false);
            kid.state_timer += dt;
            // Unlike fall speed, kUnsplatSeconds is a member variable
            // because the main function needs to know about it
            // when drawing the exclamation point sprite
            if (kid.state_timer > kid.kUnsplatSeconds) {
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            break;
        case Kid::JUMP:
            if (KidJumpUpdate(kid, ctx, kid_collision))
                break;
            if (ks.s == 0 && ks.spcp > 0) {
                kid.charge_started = true;
                kid.charge_timer += dt;
            } else if (kid.charge_started && ks.spc > 0) {
                kid.charge_timer += dt;
                kid.swing_anchor = KidRopeFindAnchor(kid, ctx, rp);
            } else if (kid.charge_started || ks.tp) {
                kid.swing_anchor = KidRopeFindAnchor(kid, ctx, rp);
                if (rp != nullptr) {
                    RopeCreateAndLink(rs, *rp, kid.pos, kRopeLength, true, kid.prev_pos);
                } else {
                    RopeCreate(rs, kid.swing_anchor, kid.pos, kRopeLength, true, false, kid.prev_pos);
                }
                KidSwitchState(kid, Kid::SWING);
                break;
            }
            if (ks.y == 1 && ks.spcp > 0) {
                // KidSwitchState(kid, Kid::PARRY);
                // break;
                // Below is the old down + spacebar code, which would
                // put the kid in the BOUNCE state.
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
            KidStarUpdate(kid, ctx, 0.4);
            KidVisualUpdate(kid, ctx, false);
            break;
        case Kid::PARRY:
            KidStarUpdate(kid, ctx, 1.1);
            KidVisualUpdate(kid, ctx, false);
            kid.state_timer += dt;
            if (kid.state_timer > kParrySeconds) {
                KidSwitchState(kid, Kid::STAND);
                break;
            }
            break;
        case Kid::CHARGE_BOUNCE:
            if (kid.state_timer >= 1.0 && kid.state == Kid::CHARGE_BOUNCE) {
                kid.speed += 75.0;
                kid.vel = V2d(ks.x, ks.y).Normalized() * kid.speed;
                KidSwitchState(kid, Kid::JUMP);
                break;
            }
            KidVisualUpdate(kid, ctx, false);
            KidStarUpdate(kid, ctx, 0.2);
            kid.state_timer += dt;
            // Alternative spin pattern for when we're not holding left or right
            if (ks.x == 0)
                kid.angle += (400.0 * signOf(ks.y) + 400.0 * ks.y) * (kid.state_timer + 1.0) * dt;
            break;
        case Kid::SWING:
            KidRopeUpdate(kid, ctx, touch_swing_acceleration_multiplier);
            KidStarUpdate(kid, ctx, 0.2);
            KidVisualUpdate(kid, ctx, false);
            if (ks_prev.spcp == 1 || touch_release_event) {
                // Web zip code here
                // ks_dir = V2d(ks.x, ks.y);
                // rs_dir = rs.rope_points[kRopePoints + kRopeLength - 1].pos - kid.pos;
                // if (ks_dir * rs_dir > 0) {
                //     kid.vel = ks_dir.Normalized() * (kid.vel.Magnitude() + (V2d(0, -100.0) * rs_dir.Normalized()));
                // }
                rs.rope_points[kRopePoints].holding_player = false;
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
