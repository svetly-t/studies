#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>
#include <algorithm>

#include "include/sdl_state.h"
#include "include/camera.h"
#include "include/v2d.h"

double Lerp(double from, double to, double factor) {
    return from + (to - from) * factor;
}

class GameState {
 public:
    enum State {
        GENERATE_TERRAIN,
        PLAY
    };
    State state;
};

class Kid {
 public:
    Kid() {
        state = State::SLIDING;
    }
    enum State {
        WALKING,
        SLIDING,
        STOP_SLIDING,
        FALLING,
        SKIDDING,
        IDLE,
    };
    State state;
    V2d pos;
    V2d vel;
    V2d acc;
};

const double kTerrainSlack = 0.0;

double terrain_function(double x) {
    return 2.0 * sin(x / 2.0);
}

double terrain_function_derivative(double x) {
    return cos(x / 2.0);
}

V2d terrain_function_tangent(double x) {
    V2d result;
    result.x = 1;
    result.y = terrain_function_derivative(x);
    return result.Normalized();
}

V2d terrain_function_normal(double x) {
    V2d result = terrain_function_tangent(x);
    double temp = result.x;
    result.x = -result.y;
    result.y = temp;
    return result;
}

int main(int argc, char **argv) {
    const int kWindowX = 800; 
    const int kWindowY = 600;
    const int kKidW = 16;
    const int kKidH = 16;
    const double kGravity = 20.0;

    float dt = 16.0 / 1000.0;

    double kKidStartX = 8;
    double kKidStartY = terrain_function(kKidStartX);

    Kid kid;
    kid.pos.x = kKidStartX;
    kid.pos.y = kKidStartY;

    GameState game_state;

    SdlState sdl_state(kWindowX, kWindowY);

    Camera camera(sdl_state.sdl_renderer, kWindowY, kWindowX);

    KeyState ks;

    // Event loop
    for (;!sdl_state.exit;) {
        sdl_state.GetEvents(ks);

        double coeff_of_friction = 0.1;
        V2d tangent = terrain_function_tangent(kid.pos.x);
        V2d normal = terrain_function_normal(kid.pos.x);
        V2d gravity(0.0, kGravity);
        V2d gravity_projected_onto_terrain =  tangent * (gravity * tangent);
        V2d gravity_projected_onto_normal =  normal * (gravity * normal);
        V2d friction_force = tangent * coeff_of_friction * gravity_projected_onto_normal.Magnitude();
        // friction should oppose the direction of movement
        if (friction_force * gravity_projected_onto_terrain > 0)
            friction_force = -friction_force;
        switch (kid.state) {
            case Kid::FALLING:
                kid.acc.x = 0;
                kid.acc.y = kGravity + (double)ks.y * kGravity;
                // kid.vel += a * dt
                kid.vel.x += kid.acc.x * dt;
                kid.vel.y += kid.acc.y * dt;
                // kid.pos += kid.vel * dt
                kid.pos.x += kid.vel.x * dt;
                kid.pos.y += kid.vel.y * dt;
                // constrain to keep the kid on the line
                if (kid.pos.y > terrain_function(kid.pos.x)) {
                    kid.pos.y = terrain_function(kid.pos.x);
                    kid.vel -= normal * (kid.vel * normal);
                    kid.state = Kid::SLIDING;
                    break;
                }
                break;
            case Kid::SLIDING:
                // acceleration is (projection of weight onto tangent of terrain curve) / mass
                kid.acc = gravity_projected_onto_terrain;
                // Add to this the projection of directional boost
                kid.acc.x += (double)ks.x * 8.0;
                kid.acc.y += (double)ks.x * 8.0 * terrain_function_derivative(kid.pos.x);
                // kid.vel += a * dt
                kid.vel.x += kid.acc.x * dt;
                kid.vel.y += kid.acc.y * dt;
                // directly kick velocity on left, right button press
                if (ks.xp) {
                    kid.vel.x += (double)ks.x * 16.0;
                    kid.vel.y += (double)ks.x * 16.0 * terrain_function_derivative(kid.pos.x);
                }
                // kid.pos += kid.vel * dt
                kid.pos.x += kid.vel.x * dt;
                kid.pos.y += kid.vel.y * dt;
                // if significantly above the ground we're falling
                if (kid.pos.y < terrain_function(kid.pos.x) - kTerrainSlack) {
                    kid.state = Kid::FALLING;
                    break;
                }
                // constrain to keep the kid on the line
                if (kid.pos.y > terrain_function(kid.pos.x)) {
                    kid.pos.y = terrain_function(kid.pos.x);
                    // kill the portion of velocity that is in the direction of the terrain
                    kid.vel -= normal * (kid.vel * normal);
                }
                // Stop sliding if spacebar not pressed
                if (ks.s == 0) {
                    kid.state = Kid::STOP_SLIDING;
                    break;
                }
                break;
            case Kid::STOP_SLIDING:
                // acceleration is (projection of weight onto tangent of terrain curve) / mass
                kid.acc = gravity_projected_onto_terrain + friction_force;
                // kid.vel += a * dt
                kid.vel.x += kid.acc.x * dt;
                kid.vel.y += kid.acc.y * dt;
                // kid.pos += kid.vel * dt
                kid.pos.x += kid.vel.x * dt;
                kid.pos.y += kid.vel.y * dt;
                // if significantly above the ground we're falling
                if (kid.pos.y < terrain_function(kid.pos.x) - kTerrainSlack) {
                    kid.state = Kid::FALLING;
                    break;
                }
                // constrain to keep the kid on the line
                if (kid.pos.y > terrain_function(kid.pos.x)) {
                    kid.pos.y = terrain_function(kid.pos.x);
                    // kill the portion of velocity that is in the direction of the terrain
                    kid.vel -= normal * (kid.vel * normal);
                }
                if (kid.vel.Magnitude() < 0.1) {
                    kid.state = Kid::IDLE;
                    kid.vel.x = 0;
                    kid.vel.y = 0;
                    break;
                }
                // Go back to sliding if spacebar not pressed
                if (ks.s == 1) {
                    kid.state = Kid::SLIDING;
                    break;
                }
                break;
            case Kid::IDLE:
                kid.pos.y = terrain_function(kid.pos.x);
                // Go back to sliding if spacebar not pressed
                if (ks.s == 1) {
                    kid.state = Kid::SLIDING;
                    break;
                }
                break;
            default:
                break;
        }

        // Move the camera so that the player is always in the center of the view window
        // Add an offset so that, plus velocity vector, we shift in the direction player is going
        // camera.pos.x = kid.pos.x + kid.vel.x;
        camera.pos.x = Lerp(camera.pos.x, kid.pos.x + kid.vel.x, dt * 2.0);

        // draw
        SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

        camera.DrawTerrain(terrain_function);

        camera.DrawBox(kid.pos);

        V2d kid_pos = camera.ToScreenSpace(kid.pos);

        // Draw debug velocity ray
        SDL_RenderDrawLine(sdl_state.sdl_renderer, kid_pos.x, kid_pos.y, kid_pos.x + kid.vel.x, kid_pos.y + kid.vel.y);

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 0, 0, 255);
        // Draw debug normal ray
        SDL_RenderDrawLine(sdl_state.sdl_renderer, kid_pos.x, kid_pos.y, kid_pos.x + normal.x * 10, kid_pos.y + normal.y * 10);
        
        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 0, 125, 255, 255);
        // Draw debug friction ray
        SDL_RenderDrawLine(sdl_state.sdl_renderer, kid_pos.x, kid_pos.y, kid_pos.x + friction_force.x, kid_pos.y + friction_force.y);

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 0, 255, 125, 255);
        // Draw debug force of gravity ray
        SDL_RenderDrawLine(sdl_state.sdl_renderer, kid_pos.x, kid_pos.y, kid_pos.x + gravity_projected_onto_terrain.x, kid_pos.y + gravity_projected_onto_terrain.y);

        SDL_UpdateWindowSurface(sdl_state.sdl_window);

        // sleep
        SDL_Delay(16);
    }

    return EXIT_SUCCESS;
}