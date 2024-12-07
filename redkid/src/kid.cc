#include "kid.h"
#include "terrain.h"
#include "sdl_state.h"

void Kid::Init(double x, double y) {
    pos.x = x;
    pos.y = y;
}

void Kid::Update(Kid::UpdateContext *ctx) {
    const double kTerrainSlack = 0.0;
    double coeff_of_friction = 0.1;
    double k_gravity = ctx->gravity;
    double dt = ctx->dt;
    double terrain_height = ctx->terrain->Height(pos.x);
    
    V2d tangent = ctx->terrain->Tangent(pos.x);
    V2d normal = ctx->terrain->Normal(pos.x);
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
            // constrain to keep the kid on the line
            if (pos.y > ctx->terrain->Height(pos.x)) {
                pos.y = ctx->terrain->Height(pos.x);
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
            acc.y += (double)ctx->ks->x * 8.0 * ctx->terrain->Slope(pos.x);
            // vel += a * dt
            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            // directly kick velocity on left, right button press
            if (ctx->ks->xp) {
                vel.x += (double)ctx->ks->x * 16.0;
                vel.y += (double)ctx->ks->x * 16.0 * ctx->terrain->Slope(pos.x);
            }
            // pos += vel * dt
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            // if significantly above the ground we're falling
            if (pos.y < ctx->terrain->Height(pos.x) - kTerrainSlack) {
                state = Kid::FALLING;
                break;
            }
            // constrain to keep the kid on the line
            if (pos.y > ctx->terrain->Height(pos.x)) {
                pos.y = ctx->terrain->Height(pos.x);
                // kill the portion of velocity that is in the direction of the terrain
                vel -= normal * (vel * normal);
            }
            // Stop sliding if spacebar not pressed
            if (ctx->ks->s == 0) {
                state = Kid::STOP_SLIDING;
                break;
            }
            break;
        case Kid::STOP_SLIDING:
            // acceleration is (projection of weight onto tangent of terrain curve) / mass
            acc = gravity_projected_onto_terrain + friction_force;
            // vel += a * dt
            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            // pos += vel * dt
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            // if significantly above the ground we're falling
            if (pos.y < ctx->terrain->Height(pos.x) - kTerrainSlack) {
                state = Kid::FALLING;
                break;
            }
            // constrain to keep the kid on the line
            if (pos.y > ctx->terrain->Height(pos.x)) {
                pos.y = ctx->terrain->Height(pos.x);
                // kill the portion of velocity that is in the direction of the terrain
                vel -= normal * (vel * normal);
            }
            if (vel.Magnitude() < 0.1) {
                state = Kid::IDLE;
                vel.x = 0;
                vel.y = 0;
                break;
            }
            // Go back to sliding if spacebar not pressed
            if (ctx->ks->s == 1) {
                state = Kid::SLIDING;
                break;
            }
            break;
        case Kid::IDLE:
            pos.y = ctx->terrain->Height(pos.x);
            // Go back to sliding if spacebar not pressed
            if (ctx->ks->s == 1) {
                state = Kid::SLIDING;
                break;
            }
            break;
        default:
            break;
    }
}