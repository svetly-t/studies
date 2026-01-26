#include "include/sdl_state.h"
#include "include/v2d.h"
#include "include/kid.h"
#include "include/kidsprite.h"
#include "include/title.h"

#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

static int kScreenWidth = 800;
static int kScreenHeight = 600;
static unsigned int kMillisecondsPerFrame = 1000/60;

struct Camera {
    V2d pos;
    // As zoom gets lower, the size of drawn elements scales linearly.
    // So a 100x100 box at 1.0 zoom becomes a 50x50 box at 0.5 times zoom.
    // Similarly, distances from origin should be scaled as well.
    double zoom = 1.0;
};

void CameraUpdate(Camera &camera, Kid &kid, KeyState &ks, V2d &mouse_pos, double dt) {
    V2d origin;
    V2d transform_screen = { (double)kScreenWidth / 2, (double)kScreenHeight / 2 };
    V2d ks_mp = { (double)ks.mx, (double)ks.my };

    if (ks.z) {
        camera.zoom += (0.5 - camera.zoom) * dt * 4.0;
    } else {
        camera.zoom += (1.0 - camera.zoom) * dt * 4.0;
    }

    camera.pos += (kid.pos - camera.pos) * dt * 2.0;
    origin = camera.pos - transform_screen;

    mouse_pos = origin + ks_mp;
}

// The width and height are the box size in pixels at zoom = 1.0.
void DrawTextureAtV2d(SdlState &sdl_state, Camera &camera, SDL_Texture *texture, SDL_Rect src, V2d pos) {
    V2d transformed_pos;
    SDL_Rect sdl_rect;
    V2d offset_to_center;
    offset_to_center.x = src.w / 2.0;
    offset_to_center.y = src.h / 2.0;
    transformed_pos = (pos - offset_to_center - camera.pos) * camera.zoom;
    sdl_rect.x = transformed_pos.x + kScreenWidth / 2;
    sdl_rect.y = transformed_pos.y + kScreenHeight / 2;
    sdl_rect.w = src.w * camera.zoom;
    sdl_rect.h = src.h * camera.zoom;
    SDL_RenderCopyEx(sdl_state.sdl_renderer, texture, &src, &sdl_rect, 0, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
}

// The width and height are the box size in pixels at zoom = 1.0.
void DrawBoxAtV2d(SdlState &sdl_state, Camera &camera, V2d pos, int width, int height) {
    V2d transformed_pos;
    SDL_Rect sdl_rect;
    transformed_pos = (pos - camera.pos) * camera.zoom;
    sdl_rect.x = transformed_pos.x + kScreenWidth / 2;
    sdl_rect.y = transformed_pos.y + kScreenHeight / 2;
    sdl_rect.w = width * camera.zoom;
    sdl_rect.h = height * camera.zoom;
    SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect);
}

void DrawAABB(SdlState &sdl_state, Camera &camera, AABB aabb) {
    DrawBoxAtV2d(sdl_state, camera, aabb.pos, aabb.width, aabb.height);
}

void DrawLine(SdlState &sdl_state, Camera &camera, V2d p1, V2d p2) {
    V2d transform_screen = { (double)kScreenWidth / 2, (double)kScreenHeight / 2 };
    p1 = (p1 - camera.pos) * camera.zoom + transform_screen;
    p2 = (p2 - camera.pos) * camera.zoom + transform_screen;
    SDL_RenderDrawLine(sdl_state.sdl_renderer, p1.x, p1.y, p2.x, p2.y);
}

// Use this to fix frametimes: https://discourse.libsdl.org/t/poor-performance-of-sdl2-on-macos/28276/3
Uint32 timerTickCallBack(Uint32 iIntervalInMilliseconds, void *param) {
    SDL_Event event;
    SDL_UserEvent userevent;

    if (SDL_HasEvent(SDL_USEREVENT) == false) {
        // add an SDL_USEREVENT to the message queue
        userevent.type = SDL_USEREVENT;
        userevent.code = 0;
        userevent.data1 = NULL;
        userevent.data2 = NULL;

        event.type = SDL_USEREVENT;
        event.user = userevent;

        SDL_PushEvent(&event);
    }

    // call back again in ‘iIntervalInMilliseconds’ milliseconds
    return iIntervalInMilliseconds;
}

struct Game {
    enum State {
        TITLE,
        DEMO
    };
    State state;

    KeyState ks_prev;
    KeyState ks;
    SdlState sdl_state;
    Level level;
    Kid kid;
    KidUpdateContext kid_update_ctx;
    KidSprite kid_sprite;
    Camera camera;
    RopeState rs;
    Title title;
    SDL_Surface *title_sprite_surface;
    SDL_Texture *title_sprite_texture;
    SDL_Surface *space_sprite_surface;
    SDL_Texture *space_sprite_texture;
    int sprite_size;

    std::chrono::steady_clock::time_point end_of_update_clock;
};

void GameSpritesInitialize(Game &game) {
    SdlSpriteLoad(game.title_sprite_surface, game.title_sprite_texture, game.sdl_state.sdl_renderer, "./img/title-simple.png");
    SdlSpriteLoad(game.space_sprite_surface, game.space_sprite_texture, game.sdl_state.sdl_renderer, "./img/press-space.png");
    game.sprite_size = 500;
}

void title(void *vgame) {
    SDL_Rect src;
    SDL_Rect dst_title;
    SDL_Rect dst_space;
    double dt;
    bool cancel = false;

    Game *game = (Game*)vgame;    
    Title &title = game->title;
    KeyState &ks = game->ks;
    SdlState &sdl_state = game->sdl_state;
    auto &end_of_update_clock = game->end_of_update_clock; 

    SdlStatePollEvents(ks, cancel);

    if (cancel) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();  /* this should "kill" the app. */
        #else
        exit(0);
        #endif
    }

    dt = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-end_of_update_clock).count() / 1000000.0;

    TitleUpdate(title, ks, dt);

    end_of_update_clock = std::chrono::steady_clock::now();

    KeyStateClearPress(ks);

    if (title.state == Title::SELECTED) {
        if (title.state_timer > 1.0) {
            game->state = Game::DEMO;
            return;
        }
    }

    // draw
    SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

    SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

    src.x = 0;
    src.y = 0;
    src.w = game->sprite_size;
    src.h = game->sprite_size;

    dst_title.x = kScreenWidth / 2 - game->sprite_size / 2;
    dst_title.y = kScreenHeight / 2 - game->sprite_size / 2;
    if (title.state == Title::NOTHING) {
        dst_title.y += 10 * sin(title.state_timer * 1 * 3.14159);
    }
    dst_title.w = game->sprite_size;
    dst_title.h = game->sprite_size;

    dst_space = dst_title;
    dst_space.y = kScreenHeight / 2 - game->sprite_size / 2;

    if (title.state == Title::NOTHING || std::fmod(title.state_timer, 0.1) > 0.05) {
        SDL_RenderCopyEx(sdl_state.sdl_renderer, game->title_sprite_texture, &src, &dst_title, 0, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
        SDL_RenderCopyEx(sdl_state.sdl_renderer, game->space_sprite_texture, &src, &dst_space, 0, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
    }

    SDL_UpdateWindowSurface(sdl_state.sdl_window);
}

void demo(void *vgame) {
    double dt;
    bool cancel = false;
    double meters_per_pixel = 0.1;
    KidUpdateContext kid_update_ctx;
    V2d mouse_pos;
    SDL_Rect src;
    Game *game = (Game*)vgame;

    KeyState &ks_prev = game->ks_prev;
    KeyState &ks = game->ks;
    SdlState &sdl_state = game->sdl_state;
    auto &end_of_update_clock = game->end_of_update_clock; 
    Level &level = game->level;
    Kid &kid = game->kid;
    KidSprite &kid_sprite = game->kid_sprite;
    Camera &camera = game->camera;
    RopeState &rs = game->rs;

    SdlStatePollEvents(ks, cancel);

    if (cancel) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();  /* this should "kill" the app. */
        #else
        exit(0);
        #endif
    }

    dt = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-end_of_update_clock).count() / 1000000.0;

    kid_update_ctx.level = &level;
    kid_update_ctx.ks = &ks;
    kid_update_ctx.rs = &rs;
    kid_update_ctx.ks_prev = &ks_prev;
    kid_update_ctx.dt = dt;
    kid_update_ctx.meters_per_pixel = meters_per_pixel;
    KidUpdate(kid, kid_update_ctx);

    KidSpriteUpdate(kid_sprite, kid, dt);

    CameraUpdate(camera, kid, ks, mouse_pos, dt);

    LevelUpdate(level, rs, ks, mouse_pos, dt);

    RopeStateUpdate(rs, dt);

    end_of_update_clock = std::chrono::steady_clock::now();

    ks_prev = ks;

    KeyStateClearPress(ks);

    // draw
    SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

    SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

    // Drawing the kid star
    // for (int i = 0; i < 4; ++i)
    //     DrawLine(sdl_state, camera, kid.visual_pos, kid.star_pos[i]);

    src.x = kid_sprite.active_size * kid_sprite.active_frame;
    src.y = kid_sprite.active_size * kid_sprite.active_vertical_index;
    src.h = src.w = kid_sprite.active_size;
    DrawTextureAtV2d(sdl_state, camera, kid_sprite.active_sprite, src, kid.visual_pos);

    // Drawing the aiming reticle
    if (kid.state == Kid::JUMP &&
        kid.charge_started &&
        kid.charge_timer > 0.0) {
        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 100, 100, 255);
        DrawBoxAtV2d(sdl_state, camera, kid.swing_reticle.pos, kid.swing_reticle.width, kid.swing_reticle.height);
        DrawLine(sdl_state, camera, kid.swing_anchor - V2d(8.0, 0.0), kid.swing_anchor + V2d(8.0, 0.0));
        DrawLine(sdl_state, camera, kid.swing_anchor - V2d(0.0, 8.0), kid.swing_anchor + V2d(0.0, 8.0));
        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);
    }

    // Drawing the kid rope
    for (int i = 0; i < kSwingPoints - 1; ++i) {
        DrawBoxAtV2d(sdl_state, camera, kid.swing_pos[i], 2, 2);
        DrawLine(sdl_state, camera, kid.swing_pos[i], kid.swing_pos[i + 1]);
    }

    // Drawing the level ropes
    for (int i = 0; i < kRopePointsTotal; ++i) {
        if (!rs.rope_points[i].active)
            continue;
        DrawBoxAtV2d(sdl_state, camera, rs.rope_points[i].pos, 2, 2);
        if (rs.rope_points[i].neighbor_idx == -1)
            continue;
        DrawLine(sdl_state, camera, rs.rope_points[rs.rope_points[i].neighbor_idx].pos, rs.rope_points[i].pos);
    }

    DrawBoxAtV2d(sdl_state, camera, mouse_pos, 2, 2);

    DrawAABB(sdl_state, camera, level.aabb);

    for (auto &aabb : level.aabbs) {
        DrawAABB(sdl_state, camera, aabb);
    }

    // Draw level test line
    DrawLine(sdl_state, camera, level.l1, level.l2);

    // Draw kid velocity line
    DrawLine(sdl_state, camera, kid.pos, kid.pos + kid.vel);

    LineToLineIntersection isct = AABBToLineIntersect(level.aabb, level.l1, level.l2);
    if (isct.exists) {
        // Draw level test line
        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 0, 0, 255);
        DrawBoxAtV2d(sdl_state, camera, isct.intersection_point, 4, 4);
        DrawBoxAtV2d(sdl_state, camera, isct.projection_point, 4, 4);
        DrawLine(sdl_state, camera, isct.projection_point, isct.projection_point + isct.normal * 8.0);
    }

    SDL_UpdateWindowSurface(sdl_state.sdl_window);
}

void gameloop(void *vgame) {
    Game *game = (Game*)vgame;
    switch (game->state) {
        case Game::TITLE:
            title(vgame);
            break;
        case Game::DEMO:
            demo(vgame);
            break;
    }
}

int main(int argc, char **argv) {
    Game game;
    game.state = Game::TITLE;
    game.end_of_update_clock = std::chrono::steady_clock::now();

    #ifdef __EMSCRIPTEN__
    // Emscripten handles the frame timing by itself
    #else
    SDL_AddTimer(kMillisecondsPerFrame, timerTickCallBack, NULL);
    #endif

    SdlStateInitialize(game.sdl_state, kScreenWidth, kScreenHeight);
    GameSpritesInitialize(game);
    KidSpriteInitialize(game.kid_sprite, game.sdl_state.sdl_renderer);
    KidInitialize(game.kid);
    LevelInitialize(game.level, kScreenWidth, kScreenHeight);
    RopeStateInitialize(game.rs);
    TitleInitialize(game.title);

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(gameloop, (void*)&game, 0, 1);
    #else
    while (1) { gameloop((void*)&game); }
    #endif
}