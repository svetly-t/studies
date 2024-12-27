#include "kid.h"
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

void Kid::Update(Kid::UpdateContext *ctx) {
    double coeff_of_friction = 0.1;
    double k_gravity = ctx->gravity;
    double dt = ctx->dt;
    double new_height;

    V2d tangent = ctx->terrainp->Tangent(pos.x);
    V2d normal = calculateNormal(pos, ctx);
    V2d new_normal;
    V2d gravity(0.0, k_gravity);
    V2d gravity_projected_onto_terrain =  tangent * (gravity * tangent);
    V2d gravity_projected_onto_normal =  normal * (gravity * normal);
    V2d friction_force = tangent * coeff_of_friction * gravity_projected_onto_normal.Magnitude();
    // friction should oppose the direction of movement
    if (friction_force * gravity_projected_onto_terrain > 0)
        friction_force = -friction_force;
    
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
            new_height = ctx->terrainp->Height(pos.x);
            // constrain to keep the kid on the line
            if (pos.y >= new_height) {
                pos.y = new_height;
                normal = calculateNormal(pos, ctx);
                vel -= normal * (vel * normal);
                state = Kid::SLIDING;
                break;
            }
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
            // directly kick velocity on left, right button press
            // if (ctx->ks->xp) {
            //     vel.x += (double)ctx->ks->x * 16.0;
            //     vel.y += (double)ctx->ks->x * 16.0 * ctx->terrainp->Slope(pos.x);
            // }
            // pos += vel * dt
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            new_height = ctx->terrainp->Height(pos.x);
            // if significantly above the ground we're falling
            if (pos.y < new_height) {
                state = Kid::FALLING;
                break;
            }
            // constrain to keep the kid on the line
            if (pos.y >= new_height) {
                pos.y = new_height;
                new_normal = calculateNormal(pos, ctx);
                // kill the portion of velocity that is in the direction of the terrain
                vel -= new_normal * (vel * new_normal);
                // getting stuck in divots. The 0.05 is arbitrary
                if (new_normal.x * normal.x < -0.05) {
                    state = Kid::STUCK;
                    vel = {0, 0};
                    break;
                }
            }
            // Stop sliding if spacebar not pressed
            if (vel.Magnitude() < 0.1 && ctx->ks->s == 0) {
                state = Kid::IDLE;
                vel = {0, 0};
                break;
            }
            break;
        case Kid::STUCK:
        case Kid::IDLE:
            pos.y = ctx->terrainp->Height(pos.x);
            // Go back to sliding if spacebar pressed
            if (ctx->ks->s == 1) {
                state = Kid::SLIDING;
                break;
            }
            break;
        default:
            break;
    }
}