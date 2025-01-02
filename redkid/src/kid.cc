#include "kid.h"
#include "utilities.h"
#include "lerp.h"
#include "terrain.h"
#include "sdl_state.h"

// https://www.boatdesign.net/threads/decent-way-of-getting-lift-coefficient-and-drag-coefficient-for-game-design.67094/#post-930696
// https://www.grc.nasa.gov/WWW/K-12/WindTunnel/Activities/lift_formula.html

void initializeFlightCtx(Kid::FlyingContext *ctx) {
    ctx->bearing = { 1, 0 };
    ctx->max_cl = 1.5;
    ctx->min_cl = 0.2;
    ctx->max_aoa = 20;
    ctx->max_aoa_cosine = std::cos(degToRad(ctx->max_aoa));
    ctx->stall_angle = 22;
    ctx->stall_angle_cosine = std::cos(degToRad(ctx->stall_angle));
}

void Kid::Initialize(double x, double y) {
    pos.x = x;
    pos.y = y;
    initializeFlightCtx(&flying_ctx);
}

V2d calculateNormal(V2d pos, Kid::UpdateContext *ctx) {
    V2d normal = ctx->terrainp->Normal(pos.x);
    return normal;
}

double calculateCl(Kid::FlyingContext *ctx, double aoa_cosine) {
    if (aoa_cosine < ctx->max_aoa_cosine)
        return LerpBetween(ctx->min_cl, ctx->max_cl, aoa_cosine, ctx->max_aoa_cosine);

    return LerpBetween(ctx->max_cl, ctx->min_cl, aoa_cosine, 1.0);
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

    // Calculate wind vector. Basically terrain tangent at ground level, and becomes flatter as you go higher
    V2d wind_tangent = tangent;
    wind_tangent.y /= std::abs(pos.y - ctx->terrainp->Height(pos.x)) + 1.0;
    wind_tangent = wind_tangent.Normalized();

    // Variables for flying
    double air_coeff;
    double air_density = 1.0;
    double wind_speed = 10.0;
    double surface_area = 1.0;
    double lift;
    double drag;
    double aoa_cosine = flying_ctx.bearing * wind_tangent;
    double aoa_sine = flying_ctx.bearing ^ wind_tangent;
    double cl = calculateCl(&flying_ctx, aoa_cosine);

    switch (state) {
        case Kid::FLYING:
            air_coeff = 0.5 * air_density * wind_speed * wind_speed * surface_area;
            lift = air_coeff * cl;
            // drag = air_coeff * cd;
            acc.x = aoa_sine * lift;
            acc.y = k_gravity - aoa_cosine * lift;
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
        case Kid::STALLING:
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
        case Kid::WALKING:
            acc = {0, 0};
            // vel += a * dt
            vel = tangent * ((V2d(0, -1) * normal) * 4.0 + 1.0) * ctx->ks->x;
            pos += vel * dt;
            // constrain to keep the kid on the line
            pos.y = ctx->terrainp->Height(pos.x);
            // Stop walking if L/R not pressed
            if (ctx->ks->x == 0) {
                state = Kid::IDLE;
                vel = {0, 0};
                break;
            }
            if (ctx->ks->y == -1) {
                state = Kid::FLYING;
                vel = {0, -5};
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
            if (ctx->ks->x) {
                state = Kid::WALKING;
                break;
            }
            break;
        default:
            break;
    }
}