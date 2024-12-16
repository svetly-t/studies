#include <iostream>
#include <algorithm>

#include "include/sdl_state.h"
#include "include/image.h"
#include "include/camera.h"
#include "include/v2d.h"
#include "include/terrain.h"
#include "include/kid.h"

double Lerp(double from, double to, double factor) {
    return from + (to - from) * factor;
}

class Game {
 public:
    enum State {
        GENERATE_TERRAIN,
        SIMULATE
    };
    State state;
};

// template <typename T>
// class Entities {
//  public:
//     uint64_t Add(T t) {
//         entities[key++] = t;
//         return key;
//     }
//     T *Get(uint64_t key) {
//         if (entities.find(key) == entities.end())
//             return nullptr;
//         return &entities[key];
//     }
//     std::unordered_map<uint64_t, T> &GetAll() {
//         return _entities;
//     }
//  private:
//     std::unordered_map<uint64_t, T> _entities;
//     uint64_t _key = 0;
// };

int main(int argc, char **argv) {
    const int kWindowX = 800; 
    const int kWindowY = 600;
    const int kKidW = 16;
    const int kKidH = 16;
    const double kGravity = 20.0;

    float dt = 16.0 / 1000.0;

    // Entities<Line> lines;

    Game game;

    SdlState sdl_state(kWindowX, kWindowY);

    Camera camera(sdl_state.sdl_renderer, kWindowY, kWindowX);

    KeyState ks;

    DefaultTerrain terrain;

    Kid kid;

    kid.Init(8, terrain.Height(8));

    game.state = Game::SIMULATE;

    // Map drawing stuff, move into the cartographer later
    V2d cursor_pos;

    const size_t kMarkers = 100;
    size_t markers_idx = 0;
    SDL_Point markers[kMarkers];

    // Event loop
    for (;!sdl_state.exit;) {
        sdl_state.GetEvents(ks);

        Kid::UpdateContext kid_ctx;

        switch (game.state) {
            case Game::GENERATE_TERRAIN:
                cursor_pos.x = ks.mx;
                cursor_pos.y = ks.my;
                if (ks.mlcp) {
                    if (markers_idx < kMarkers) {
                        markers[markers_idx].x = cursor_pos.x;
                        markers[markers_idx].y = cursor_pos.y;
                        ++markers_idx;
                    }
                } else if (ks.mrcp) {
                    if (markers_idx > 0)
                        --markers_idx;
                }
                break;
            case Game::SIMULATE:
                kid_ctx.dt = dt;
                kid_ctx.gravity = kGravity;
                kid_ctx.ks = &ks;
                kid_ctx.terrain = &terrain;
                kid.Update(&kid_ctx);
                // Move the camera so that the player is always in the center of the view window
                // Add an offset so that, plus velocity vector, we shift in the direction player is going
                camera.pos.x = Lerp(camera.pos.x, kid.pos.x + kid.vel.x, dt * 2.0);
                break;
            default:
                break;
        }

        // draw
        SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

        switch (game.state) {
            case Game::GENERATE_TERRAIN:
                // terrain_generator.Update(&ter_gen_ctx);
                // camera.Draw(map_line);
                break;
            case Game::SIMULATE:
                camera.DrawTerrain(terrain);
                camera.DrawBox(kid.pos);
                break;
            default:
                break;
        }


        V2d debug_kid_pos = camera.ToScreenSpace(kid.pos);

        // Draw debug velocity ray
        SDL_RenderDrawLine(sdl_state.sdl_renderer, debug_kid_pos.x, debug_kid_pos.y, debug_kid_pos.x + kid.vel.x, debug_kid_pos.y + kid.vel.y);

        // SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 0, 0, 255);
        // Draw debug normal ray
        // SDL_RenderDrawLine(sdl_state.sdl_renderer, kid_pos.x, kid_pos.y, kid_pos.x + normal.x * 10, kid_pos.y + normal.y * 10);
        
        // SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 0, 125, 255, 255);
        // Draw debug friction ray
        // SDL_RenderDrawLine(sdl_state.sdl_renderer, kid_pos.x, kid_pos.y, kid_pos.x + friction_force.x, kid_pos.y + friction_force.y);

        // SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 0, 255, 125, 255);
        // Draw debug force of gravity ray
        // SDL_RenderDrawLine(sdl_state.sdl_renderer, kid_pos.x, kid_pos.y, kid_pos.x + gravity_projected_onto_terrain.x, kid_pos.y + gravity_projected_onto_terrain.y);

        SDL_UpdateWindowSurface(sdl_state.sdl_window);

        // sleep
        SDL_Delay(16);
    }

    return EXIT_SUCCESS;
}