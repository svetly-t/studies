#include "kid.h"
#include "utilities.h"
#include "lerp.h"
#include "terrain.h"
#include "sdl_state.h"

void Kid::Initialize(double x, double y) {
    pos.x = x;
    pos.y = y;
}

V2d calculateNormal(V2d pos, Kid::UpdateContext *ctx) {
    V2d normal = ctx->terrainp->Normal(pos.x);
    return normal;
}

bool doCollision(Kid::UpdateContext *ctx, Kid::State &state, V2d normal, V2d &pos, V2d &vel) {
    double height = ctx->terrainp->Height(pos.x);

    V2d new_normal = calculateNormal(pos, ctx);

    if (pos.y >= height) {
        pos.y = height;
        // kill the portion of velocity that is in the direction of the terrain
        vel -= new_normal * (vel * new_normal);
        // getting stuck in divots. The 0.05 is arbitrary
        if (new_normal.x * normal.x < -0.05) {
            state = Kid::BECOME_STUCK;
            return true;
        }
        state = Kid::SLIDING;
        return true;
    }

    return false;
}

void Kid::Update(Kid::UpdateContext *ctx) {
    double coeff_of_friction = 0.1;
    double k_gravity = ctx->gravity;
    double dt = ctx->dt;

    V2d tangent = ctx->terrainp->Tangent(pos.x);
    V2d normal = calculateNormal(pos, ctx);
    V2d gravity(0.0, k_gravity);
    V2d gravity_projected_onto_terrain =  tangent * (gravity * tangent);
    V2d gravity_projected_onto_normal =  normal * (gravity * normal);
    V2d friction_force = tangent * coeff_of_friction * gravity_projected_onto_normal.Magnitude();
    // friction should oppose the direction of movement
    if (friction_force * gravity_projected_onto_terrain > 0)
        friction_force = -friction_force;

    // Flip sprite based on x
    if (ctx->ks->x != 0)
        *ctx->sprite_flip = ctx->ks->x;

    switch (state) {
        case Kid::FALLING:
            acc.x = 0;
            acc.y = k_gravity + (double)ctx->ks->y * k_gravity;
            // vel += a * dt
            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            // pos += vel * dt
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            // switch sprite if significantly above the ground
            if (pos.y + 5.0 < ctx->terrainp->Height(pos.x))
                *ctx->sprite_frame = 5;
            // constrain to keep the kid on the line
            if (doCollision(ctx, state, normal, pos, vel))
                break;
            break;
        case Kid::SLIDING:
            // acceleration is (projection of weight onto tangent of terrain curve) / mass
            acc = gravity_projected_onto_terrain;
            // Add to this the projection of directional boost
            acc.x += (double)ctx->ks->x * 8.0;
            acc.y += (double)ctx->ks->x * 8.0 * ctx->terrainp->Slope(pos.x);
            // vel += a * dt
            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            // pos += vel * dt
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            state_ctx.timer += ctx->dt;
            if (vel.Magnitude() < 4.0) {
                *ctx->sprite_frame = 4;
            } else if (state_ctx.timer > 2.0 / vel.Magnitude()) {
                switch (*ctx->sprite_frame) {
                    case 3:
                        *ctx->sprite_frame = 4;
                        break;
                    case 4:
                    default:
                        *ctx->sprite_frame = 3;
                        break;
                }
                state_ctx.timer = 0;
            }
            // if significantly above the ground we're falling
            if (pos.y < ctx->terrainp->Height(pos.x)) {
                state = Kid::FALLING;
                break;
            }
            // Stop sliding if spacebar not pressed
            if (vel.Magnitude() < 0.1 && ctx->ks->s == 0) {
                state = Kid::BECOME_IDLE;
                break;
            }
            // constrain to keep the kid on the line
            if (doCollision(ctx, state, normal, pos, vel))
                break;
            break;
        case Kid::AUTO_WALKING:
            // If X is being pressed, don't autowalk
            if (ctx->ks->x) {
                state = Kid::WALKING;
                break;
            }
            // Spoof the X button being held
            ctx->ks->x = state_ctx.last_held_x;
        case Kid::WALKING:
            vel = tangent * ((V2d(0, -1) * normal) * 4.0 + 1.0) * ctx->ks->x;
            pos += vel * dt;
            // constrain to keep the kid on the line
            pos.y = ctx->terrainp->Height(pos.x);
            // Stop walking if L/R not pressed
            if (ctx->ks->x == 0) {
                state = Kid::BECOME_IDLE;
                break;
            }
            state_ctx.last_held_x = ctx->ks->x;
            // Alternate between walk sprites based on timer
            state_ctx.timer += ctx->dt;
            if (state_ctx.timer > 0.25) {
                switch (*ctx->sprite_frame) {
                    case 1:
                        *ctx->sprite_frame = 2;
                        break;
                    case 2:
                    default:
                        *ctx->sprite_frame = 1;
                        break;
                }
                state_ctx.timer = 0;
            }
            break;
        case Kid::BECOME_STUCK:
        case Kid::BECOME_IDLE:
            state_ctx.timer = 0;
            *ctx->sprite_frame = 0;
            vel = {0, 0};
            state = Kid::IDLE;
            break;
        case Kid::STUCK:
        case Kid::IDLE:
            pos.y = ctx->terrainp->Height(pos.x);
            // Go back to sliding if spacebar pressed
            if (ctx->ks->s == 1) {
                state = Kid::SLIDING;
                break;
            }
            // Start walking on x button
            if (ctx->ks->x) {
                state = Kid::WALKING;
                break;
            }
            // Start walking if we've been idling > 30s
            state_ctx.timer += ctx->dt;
            if (state_ctx.timer > 10.0) {
                state = Kid::AUTO_WALKING;
                break;
            }
            break;
        default:
            break;
    }
}