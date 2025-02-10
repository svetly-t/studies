#include <iostream>
#include <algorithm>

#include "include/sdl_state.h"
#include "include/image.h"
#include "include/camera.h"
#include "include/v2d.h"
#include "include/terrain.h"
#include "include/terrain_builder.h"
#include "include/kid.h"
#include "include/lerp.h"

class Game {
 public:
    enum State {
        INITIALIZE_GENERATE_TERRAIN,
        SWITCH_TO_GENERATE_TERRAIN,
        GENERATE_TERRAIN,
        INITIALIZE_SIMULATE,
        SWITCH_TO_SIMULATE,
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

    Game game;

    SdlState sdl_state(kWindowX, kWindowY);

    Camera camera(sdl_state.sdl_renderer, kWindowY, kWindowX);

    KeyState ks;

    // Sprite stuff is simple stupid because I can't think of an abstraction
    SDL_Surface *kid_sprite_surface = IMG_Load("./img/sprite_sheet.png");

    SDL_Texture *kid_sprite_texture = SDL_CreateTextureFromSurface(sdl_state.sdl_renderer, kid_sprite_surface);

    int kid_sprite_frame = 0;

    int kid_sprite_flip = 1;

    // Each kid sprite is 135 x 135 measured by hand
    const int kKidSpriteSize = 135;

    Terrain *terrainp = nullptr;

    TerrainBuilder terrain_builder;

    Kid kid;

    // Leg right_leg(Leg::STANDING);

    // Leg left_leg(Leg::SWINGING);

    game.state = Game::INITIALIZE_GENERATE_TERRAIN;

    V2d cursor_pos;

    V2d world_dimensions;

    V2d kid_vel_normalized;

    // Event loop
    for (;!sdl_state.exit;) {
        sdl_state.GetEvents(ks);

        Kid::UpdateContext kid_ctx;

        Leg::UpdateContext leg_ctx;

        TerrainBuilder::UpdateContext terrain_builder_ctx;

        switch (game.state) {
            case Game::INITIALIZE_GENERATE_TERRAIN:
                terrain_builder.Initialize(1000, 1.0);
                terrainp = terrain_builder.GetTerrain();
                game.state = Game::SWITCH_TO_GENERATE_TERRAIN;
                break;
            case Game::SWITCH_TO_GENERATE_TERRAIN:
                camera.SetZoom(1.0);
                world_dimensions = camera.ToWorldSpace({0, 0});
                game.state = Game::GENERATE_TERRAIN;
                break;
            case Game::GENERATE_TERRAIN:
                terrain_builder_ctx.ks = &ks;
                terrain_builder_ctx.camerap = &camera;
                terrain_builder.Update(&terrain_builder_ctx);
                camera.pos.x = Lerp(camera.pos.x, kid.pos.x + kid.vel.x, dt * 2.0);
                if (ks.escp)
                    game.state = Game::INITIALIZE_SIMULATE;
                break;
            case Game::INITIALIZE_SIMULATE:
                kid.Initialize(8, terrainp->Height(8));
                game.state = Game::SWITCH_TO_SIMULATE;
                break;
            case Game::SWITCH_TO_SIMULATE:
                camera.SetZoom(0.1);
                game.state = Game::SIMULATE;
                break;
            case Game::SIMULATE:
                kid_ctx.dt = dt;
                kid_ctx.gravity = kGravity;
                kid_ctx.ks = &ks;
                kid_ctx.terrainp = terrainp;
                kid_ctx.sprite_frame = &kid_sprite_frame;
                kid_ctx.sprite_flip = &kid_sprite_flip;
                kid.Update(&kid_ctx);
                // Move the camera so that the player is always in the center of the view window
                // Add an offset so that, plus velocity vector, we shift in the direction player is going
                kid_vel_normalized = kid.vel.Normalized();
                camera.pos.x = Lerp(camera.pos.x, kid.pos.x + kid_vel_normalized.x, dt * 4.0);
                camera.pos.y = Lerp(camera.pos.y, kid.pos.y + kid_vel_normalized.y, dt * 4.0);
                // Pull back the camera back based on kid height
                // camera.SetZoom(LerpBetween(0.1, 1.0, std::abs(kid.pos.y), std::abs(world_dimensions.y)));
                // Pull back the camera if E is held
                if (ks.e)
                    camera.SetZoom(Lerp(camera.GetZoom(), 1.0, dt));
                else
                    camera.SetZoom(Lerp(camera.GetZoom(), 0.1, dt * 4.0));
                // Switch to generate terrain on ESC pressed
                if (ks.escp)
                   game.state = Game::SWITCH_TO_GENERATE_TERRAIN;
                break;
            default:
                break;
        }

        sdl_state.ClearPress(ks);

        // draw
        SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

        switch (game.state) {
            case Game::GENERATE_TERRAIN:
                cursor_pos.x = ks.mx;
                cursor_pos.y = ks.my;
                camera.DrawTerrain(terrainp);
                camera.DrawCursor(cursor_pos);
                break;
            case Game::SIMULATE:
                camera.DrawTerrain(terrainp);
                camera.DrawSprite(kid.pos, kid_sprite_texture, kKidSpriteSize, kid_sprite_frame, 32 * kid_sprite_flip, 16);
                break;
            default:
                break;
        }


        V2d debug_kid_pos = camera.ToScreenSpace(kid.pos);
        V2d debug_normal = terrainp->Normal(kid.pos.x);

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 0, 0, 255, 255);
        // Draw debug velocity ray
        SDL_RenderDrawLine(sdl_state.sdl_renderer, debug_kid_pos.x, debug_kid_pos.y, debug_kid_pos.x + kid.vel.x, debug_kid_pos.y + kid.vel.y);

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 0, 0, 255);
        // Draw debug normal ray
        SDL_RenderDrawLine(sdl_state.sdl_renderer, debug_kid_pos.x, debug_kid_pos.y, debug_kid_pos.x + debug_normal.x * 10, debug_kid_pos.y + debug_normal.y * 10);

        SDL_UpdateWindowSurface(sdl_state.sdl_window);

        // sleep
        SDL_Delay(16);
    }

    return EXIT_SUCCESS;
}