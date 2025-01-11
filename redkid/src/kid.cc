#include "kid.h"
#include "utilities.h"
#include "lerp.h"
#include "terrain.h"
#include "sdl_state.h"

void initializeFlightCtx(Kid::FlyingContext *ctx) {
    ctx->angle = 0;
    ctx->max_cl = 1.5;
    ctx->min_cl = 0.2;
    ctx->max_aoa = 30;
    ctx->max_aoa_cosine = std::cos(degToRad(ctx->max_aoa));
    ctx->stall_angle = 45;
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

// See an example of a Cl curve here:
//      https://www.researchgate.net/figure/Cl-curves-for-the-NACA-63-3-618-and-the-NACA-4415-airfoil-Re-13-10-6-from-XFOIL_fig76_307960051
//      https://www.boatdesign.net/threads/decent-way-of-getting-lift-coefficient-and-drag-coefficient-for-game-design.67094/#post-930696
//      https://www.grc.nasa.gov/WWW/K-12/WindTunnel/Activities/lift_formula.html
// We emulate this with a piecewise linear function. Also instead of using angles we use cosine(angle), because we can get those from the dot product
double calculateCl(Kid::FlyingContext *ctx, double aoa_cosine) {
    if (aoa_cosine < ctx->max_aoa_cosine)
        return LerpBetween(ctx->min_cl, ctx->max_cl, aoa_cosine, ctx->max_aoa_cosine);

    return LerpBetween(ctx->max_cl, ctx->min_cl, aoa_cosine, 1.0);
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
            vel = {0, 0};
            state = Kid::STUCK;
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

    // Make up a wind vector. Basically terrain tangent at ground level, and becomes flatter as you go higher
    V2d wind_tangent = tangent;
    // wind_tangent.y /= std::abs(pos.y - ctx->terrainp->Height(pos.x)) + 1.0;
    wind_tangent = wind_tangent.Normalized();

    // Bearing based on flight angle
    V2d bearing(std::cos(flying_ctx.angle), -std::sin(flying_ctx.angle));

    // Variables for flying
    double air_coeff;
    double wind_speed = 7.2; // + vel * wind_tangent;
    double lift;
    double drag;
    double aoa_cosine = bearing * wind_tangent;
    double aoa_sine = std::abs(bearing ^ wind_tangent);
    double cl = calculateCl(&flying_ctx, aoa_cosine);

    switch (state) {
        case Kid::FLYING:
            // Technically this is the formula for the non-cl coefficient:
            //      air_coeff = 0.5 * air_density * wind_speed * wind_speed * surface_area;
            // But these are all relative constants, except for wind speed. So we might as well condense the other three
            // into a magic, tweakable number.
            if (flying_ctx.angle < 0)
                cl = -cl;
            air_coeff = 0.5 * wind_speed * wind_speed;
            lift = air_coeff * cl;
            // drag = air_coeff * cd;
            acc.x = aoa_sine * lift;
            acc.y = k_gravity - aoa_cosine * lift;
            // acc += bearing * ctx->ks->s * 10;
            // vel += a * dt
            vel.x += acc.x * dt;
            vel.y += acc.y * dt;
            // pos += vel * dt
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            flying_ctx.angle += dt * -ctx->ks->x;
            if (doCollision(ctx, state, normal, pos, vel))
                break;
            if (aoa_cosine < flying_ctx.stall_angle_cosine) {
               state = Kid::STALLING;
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
            // directly kick velocity on left, right button press
            // if (ctx->ks->xp) {
            //     vel.x += (double)ctx->ks->x * 16.0;
            //     vel.y += (double)ctx->ks->x * 16.0 * ctx->terrainp->Slope(pos.x);
            // }
            // pos += vel * dt
            pos.x += vel.x * dt;
            pos.y += vel.y * dt;
            // if significantly above the ground we're falling
            if (pos.y < ctx->terrainp->Height(pos.x)) {
                state = Kid::FALLING;
                break;
            }
            // Stop sliding if spacebar not pressed
            if (vel.Magnitude() < 0.1 && ctx->ks->s == 0) {
                state = Kid::IDLE;
                vel = {0, 0};
                break;
            }
            // constrain to keep the kid on the line
            if (doCollision(ctx, state, normal, pos, vel))
                break;
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
                vel += {0, -20};
                flying_ctx.angle = 0;
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