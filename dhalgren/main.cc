#include "include/sdl_state.h"
#include "include/v2d.h"
#include "include/kid.h"
#include "include/kidsprite.h"
#include "include/title.h"

#include <chrono>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const static int kWindowedWidth = 800;
const static int kWindowedHeight = 600;
static int screenWidth = 800;
static int screenHeight = 600;
static unsigned int kMillisecondsPerFrame = 1000/60;

struct Camera {
    V2d pos;
    // As zoom gets lower, the size of drawn elements scales linearly.
    // So a 100x100 box at 1.0 zoom becomes a 50x50 box at 0.5 times zoom.
    // Similarly, distances from origin should be scaled as well.
    double zoom = 1.0;
};

V2d CalculateMousePosInWorld(Camera &camera, KeyState &ks, double dt) {
    V2d origin;
    V2d transform_screen = { (double)screenWidth / 2, (double)screenHeight / 2 };
    V2d ks_mp = { (double)ks.mx, (double)ks.my };

    origin = camera.pos - transform_screen;

    return origin + ks_mp;
}

void CameraUpdate(Camera &camera, Kid &kid, KeyState &ks, double dt) {
    if (ks.z) {
        camera.zoom += (0.5 - camera.zoom) * dt * 4.0;
    } else {
        camera.zoom += (1.0 - camera.zoom) * dt * 4.0;
    }

    camera.pos += (kid.pos - camera.pos) * dt * 2.0;
}

struct Cursor {
    V2d initial_pos;
    V2d pos;
    double radius;
    double timer;
};

void CursorUpdate(Cursor &cursor, Camera &camera, KeyState &ks, double dt) {
    V2d mouse_pos = CalculateMousePosInWorld(camera, ks, dt);

    if (ks.mlc == 0) {
        cursor.timer = 0;
        cursor.radius = 50.0;
    } else if (ks.mlcp) {
        cursor.initial_pos = mouse_pos;
    } else if (ks.mlc) {
        // cursor.pos = cursor.initial_pos + (mouse_pos - cursor.initial_pos) * 0.9;
        cursor.pos = mouse_pos;
        cursor.timer += dt;
        cursor.radius = 50.0 + 200.0 / (cursor.timer * 50.0 + 1.0);
    }
}

struct DrawTexture{
    enum Space {
        WORLDSPACE,
        SCREENSPACE,
    };
    Space space = Space::WORLDSPACE;
    SDL_Texture *texture = nullptr;
    SDL_Rect src;
    SDL_Rect dst;
    V2d pos;
    double angle = 0;
    bool flip = false;
};

// The width and height are the box size in pixels at zoom = 1.0.
void DrawTextureAtV2d(SdlState &sdl_state, Camera &camera, DrawTexture draw_texture) {
    SDL_RendererFlip renderer_flip;
    V2d transformed_pos;
    V2d offset_to_center;

    SDL_Rect &src = draw_texture.src;
    SDL_Rect &dst = draw_texture.dst;
    V2d &pos = draw_texture.pos;
    double &angle = draw_texture.angle;
    bool &flip = draw_texture.flip;
    
    switch (draw_texture.space) {
        case DrawTexture::WORLDSPACE:
            offset_to_center.x = dst.w / 2.0;
            offset_to_center.y = dst.h / 2.0;
            transformed_pos = (pos - offset_to_center - camera.pos) * camera.zoom;
            dst.x = transformed_pos.x + screenWidth / 2;
            dst.y = transformed_pos.y + screenHeight / 2;
            dst.w = dst.w * camera.zoom;
            dst.h = dst.h * camera.zoom;
            break;
        case DrawTexture::SCREENSPACE:
            offset_to_center.x = dst.w / 2.0;
            offset_to_center.y = dst.h / 2.0;
            transformed_pos = (pos - offset_to_center);
            dst.x = transformed_pos.x + screenWidth / 2;
            dst.y = transformed_pos.y + screenHeight / 2;
            break;
    }

    if (draw_texture.flip) {
        renderer_flip = SDL_RendererFlip::SDL_FLIP_NONE;
    } else if (!draw_texture.flip) {
        renderer_flip = SDL_RendererFlip::SDL_FLIP_HORIZONTAL;
    }

    SDL_RenderCopyEx(sdl_state.sdl_renderer, draw_texture.texture, &src, &dst, angle, nullptr, renderer_flip);
}

// The width and height are the box size in pixels at zoom = 1.0.
void DrawBoxAtV2d(SdlState &sdl_state, Camera &camera, V2d pos, int width, int height, bool filled) {
    SDL_Rect sdl_rect;
    V2d transformed_pos = (pos - camera.pos) * camera.zoom;
    sdl_rect.x = transformed_pos.x + screenWidth / 2;
    sdl_rect.y = transformed_pos.y + screenHeight / 2;
    sdl_rect.w = width * camera.zoom;
    sdl_rect.h = height * camera.zoom;
    if (filled) {
        SDL_RenderFillRect(sdl_state.sdl_renderer, &sdl_rect);
        return;
    }
    SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect);
}

void DrawAABB(SdlState &sdl_state, Camera &camera, AABB aabb) {
    DrawBoxAtV2d(sdl_state, camera, aabb.pos, aabb.width, aabb.height, false);
}

void DrawCircle(SdlState &sdl_state, Camera &camera, Circle circle) {
    DrawBoxAtV2d(sdl_state, camera, circle.pos, circle.radius, circle.radius, true);
}

void DrawLine(SdlState &sdl_state, Camera &camera, V2d p1, V2d p2) {
    V2d transform_screen = { (double)screenWidth / 2, (double)screenHeight / 2 };
    p1 = (p1 - camera.pos) * camera.zoom + transform_screen;
    p2 = (p2 - camera.pos) * camera.zoom + transform_screen;
    SDL_RenderDrawLine(sdl_state.sdl_renderer, p1.x, p1.y, p2.x, p2.y);
}

void DrawExclamationPointAtV2d(SdlState &sdl_state, Camera &camera, V2d pos) {
    V2d p1 = pos - V2d(8.0, 8.0);
    V2d p2 = pos - V2d(4.0, 4.0);
    DrawLine(sdl_state, camera, p1, p2);
    DrawBoxAtV2d(sdl_state, camera, pos, 1.0, 1.0, false);
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
    // KidSprite kid_sprite;
    Camera camera;
    RopeState rs;
    Cursor cursor;
    Title title;

    SDL_Surface *title_sprite_surface = nullptr;
    SDL_Texture *title_sprite_texture = nullptr;
    SDL_Surface *space_sprite_surface = nullptr;
    SDL_Texture *space_sprite_texture = nullptr;
    SDL_Surface *cursor_sprite_surface = nullptr;
    SDL_Texture *cursor_sprite_texture = nullptr;
    SDL_Surface *exclamation_sprite_surface = nullptr;
    SDL_Texture *exclamation_sprite_texture = nullptr;
    SDL_Texture *dither_texture = nullptr;
    int sprite_size;

    std::chrono::steady_clock::time_point end_of_update_clock;
};

void GameSpritesInitialize(Game &game) {
    SdlSpriteLoad(game.title_sprite_surface, game.title_sprite_texture, game.sdl_state.sdl_renderer, "./img/title-simple.png");
    SdlSpriteLoad(game.space_sprite_surface, game.space_sprite_texture, game.sdl_state.sdl_renderer, "./img/press-space.png");
    SdlSpriteLoad(game.cursor_sprite_surface, game.cursor_sprite_texture, game.sdl_state.sdl_renderer, "./img/circle.png");
    SdlSpriteLoad(game.exclamation_sprite_surface, game.exclamation_sprite_texture, game.sdl_state.sdl_renderer, "./img/exclamation.png");
    game.sprite_size = 500;
}

void GameDitherInitialize(Game &game) {
    const int kPointsY = 5;
    const int kPointsX = 5;
    const int kPointCount = kPointsX * kPointsY;

    int pattern[kPointsY][kPointsX] = {
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {0, 1, 1, 1, 1}
    };

    int x;
    int y;
    int idx;

    SDL_Point points[kPointCount];

    // Create a dithering texture
    // See this post for an explanation of how to create a transparent texture https://stackoverflow.com/a/24242631
    // See this post for initializing a vector in c++11 https://stackoverflow.com/a/17663236
    // game.dither_texture = SDL_CreateTextureFromSurface(game.sdl_state.sdl_renderer, game.sdl_state.sdl_surface);
    // if (!game.dither_texture) {
    //     abort();
    // }
    game.dither_texture = SDL_CreateTexture(
        game.sdl_state.sdl_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        screenWidth,
        screenHeight
    );
    SDL_SetTextureBlendMode(game.dither_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(game.sdl_state.sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(game.sdl_state.sdl_renderer, game.dither_texture);
    SDL_SetRenderDrawBlendMode(game.sdl_state.sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(game.sdl_state.sdl_renderer, 0, 0, 0, 0);
    SDL_RenderClear(game.sdl_state.sdl_renderer);
    SDL_SetRenderDrawColor(game.sdl_state.sdl_renderer, 0, 0, 0, 255);
    for (int i = 0; i < screenHeight; i += kPointsY) {
        for (int j = 0; j < screenWidth; j += kPointsX) {
            idx = 0;
            for (int c = 0; c < kPointCount; ++c) {
                y = c / kPointsX;
                x = c % kPointsX;
                if (pattern[y][x]) {
                    points[idx++] = SDL_Point{ .x = (j + x), .y = (i + y) };
                }
            }
            SDL_RenderDrawPoints(game.sdl_state.sdl_renderer, points, idx);
        }
    }
    SDL_SetRenderTarget(game.sdl_state.sdl_renderer, nullptr);
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

    dst_title.x = screenWidth / 2 - game->sprite_size / 2;
    dst_title.y = screenHeight / 2 - game->sprite_size / 2;
    if (title.state == Title::NOTHING) {
        dst_title.y += 10 * sin(title.state_timer * 1 * 3.14159);
    }
    dst_title.w = game->sprite_size;
    dst_title.h = game->sprite_size;

    dst_space = dst_title;
    dst_space.y = screenHeight / 2 - game->sprite_size / 2;

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
    DrawTexture texture;
    V2d mouse_pos;
    V2d rnd_pos;
    SDL_Rect src;
    SDL_Rect dst;
    SDL_Rect display;
    Game *game = (Game*)vgame;

    KeyState &ks_prev = game->ks_prev;
    KeyState &ks = game->ks;
    SdlState &sdl_state = game->sdl_state;
    auto &end_of_update_clock = game->end_of_update_clock; 
    Level &level = game->level;
    Kid &kid = game->kid;
    // KidSprite &kid_sprite = game->kid_sprite;
    Camera &camera = game->camera;
    RopeState &rs = game->rs;
    Cursor &cursor = game->cursor;

    SdlStatePollEvents(ks, cancel);

    if (cancel) {
        #ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();  /* this should "kill" the app. */
        #else
        exit(0);
        #endif
    }

    dt = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-end_of_update_clock).count() / 1000000.0;

    RopeStateUpdate(rs, dt);

    kid_update_ctx.level = &level;
    kid_update_ctx.ks = &ks;
    kid_update_ctx.rs = &rs;
    kid_update_ctx.ks_prev = &ks_prev;
    kid_update_ctx.dt = dt;
    kid_update_ctx.meters_per_pixel = meters_per_pixel;
    kid_update_ctx.mouse_pos = CalculateMousePosInWorld(camera, ks, dt);
    KidUpdate(kid, kid_update_ctx);

    // KidSpriteUpdate(kid_sprite, kid, dt);

    CameraUpdate(camera, kid, ks, dt);

    LevelUpdate(level, rs, ks, CalculateMousePosInWorld(camera, ks, dt), dt);

    CursorUpdate(cursor, camera, ks, dt);

    if (ks.mlcp) {
        if (V2d(ks.mx, ks.my).SqrMagnitude() < 40.0 * 40.0) {
            sdl_state.fullscreen_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
            if (sdl_state.fullscreen_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                SDL_GetDisplayBounds(0, &display);
                screenHeight = display.h;
                screenWidth = display.w;
            } else {
                screenHeight = kWindowedHeight;
                screenWidth = kWindowedWidth;
            }
            SDL_SetWindowSize(sdl_state.sdl_window, screenWidth, screenHeight);

            SDL_SetWindowFullscreen(sdl_state.sdl_window, sdl_state.fullscreen_flags);

            #ifdef __EMSCRIPTEN__
            // Resize the window after fullscreen only if we're running on desktop
            #else
            SDL_SetWindowSize(sdl_state.sdl_window, screenWidth, screenHeight);
            #endif

            sdl_state.sdl_surface = SDL_GetWindowSurface(sdl_state.sdl_window);
            if (!sdl_state.sdl_surface) {
                abort();
            }

            SDL_DestroyRenderer(sdl_state.sdl_renderer);
            sdl_state.sdl_renderer = SDL_CreateSoftwareRenderer(sdl_state.sdl_surface);
            //SDL_RenderSetLogicalSize(sdl_state.sdl_renderer, screenWidth, screenHeight);

            GameSpritesInitialize(*game);
            GameDitherInitialize(*game);
        }
    }

    end_of_update_clock = std::chrono::steady_clock::now();

    ks_prev = ks;

    KeyStateClearPress(ks);

    // draw
    SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

    SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);
    
    for (auto &circle : level.circles) {
        DrawCircle(sdl_state, camera, circle);
    }
    
    // Drawing the dither on top of the clouds
    {
        texture.src.x = texture.src.y = 0;
        texture.src.w = screenWidth;
        texture.src.h = screenHeight;
        texture.dst = texture.src;
        texture.pos = V2d(0, 0);
        texture.texture = game->dither_texture;
        texture.space = DrawTexture::SCREENSPACE;
        DrawTextureAtV2d(sdl_state, camera, texture);
    }

    // Drawing the kid star
    switch (kid.state) {
        case Kid::SPLAT:
            texture.src.x = texture.src.y = 0;
            texture.src.h = texture.src.w = 64;
            texture.dst.h = texture.dst.w = 14;
            rnd_pos.x = rand() % 4;
            rnd_pos.y = rand() % 4;
            rnd_pos /= kid.state_timer + 1.0;
            texture.pos = kid.visual_pos + rnd_pos;
            texture.texture = game->exclamation_sprite_texture;
            texture.space = DrawTexture::WORLDSPACE;
            DrawTextureAtV2d(sdl_state, camera, texture);
            break;
        default:
            for (int i = 0; i < 4; ++i)
                DrawLine(sdl_state, camera, kid.visual_pos, kid.star_pos[i]);
            break;
    }

    // src.x = kid_sprite.size_src * kid_sprite.frame_idx;
    // src.y = kid_sprite.size_src * kid_sprite.vertical_idx;
    // src.h = src.w = kid_sprite.size_src;
    // dst.h = dst.w = kid_sprite.size_dst;
    // DrawTextureAtV2d(sdl_state, camera, kid_sprite.active_sprite, src, dst, kid.vel.x < 0.0, kid.visual_angle, kid.visual_pos);

    // Draw a cursor where the mouse pointer is at
    if (ks.mlc) {
        texture.src.x = texture.src.y = 0;
        texture.src.h = texture.src.w = game->sprite_size;
        texture.dst.h = texture.dst.w = cursor.radius * 2;
        texture.pos = cursor.pos;
        texture.texture = game->cursor_sprite_texture;
        texture.space = DrawTexture::WORLDSPACE;
        DrawTextureAtV2d(sdl_state, camera, texture);
    }

    // Drawing the aiming reticle
    if (kid.state == Kid::JUMP &&
        kid.charge_started &&
        kid.charge_timer > 0.0) {
        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 100, 100, 255);
        DrawBoxAtV2d(sdl_state, camera, kid.swing_reticle.pos, kid.swing_reticle.width, kid.swing_reticle.height, false);
        DrawLine(sdl_state, camera, kid.swing_anchor - V2d(8.0, 0.0), kid.swing_anchor + V2d(8.0, 0.0));
        DrawLine(sdl_state, camera, kid.swing_anchor - V2d(0.0, 8.0), kid.swing_anchor + V2d(0.0, 8.0));
        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);
    }

    // Drawing the kid rope
    for (int i = 0; i < kSwingPoints - 1; ++i) {
        DrawBoxAtV2d(sdl_state, camera, kid.swing_pos[i], 2, 2, false);
        DrawLine(sdl_state, camera, kid.swing_pos[i], kid.swing_pos[i + 1]);
    }

    // Drawing the level ropes
    for (int i = 0; i < kRopePointsTotal; ++i) {
        if (!rs.rope_points[i].active)
            continue;
        DrawBoxAtV2d(sdl_state, camera, rs.rope_points[i].pos, 2, 2, false);
        if (rs.rope_points[i].prev_neighbor_idx == -1)
            continue;
        DrawLine(sdl_state, camera, rs.rope_points[rs.rope_points[i].prev_neighbor_idx].pos, rs.rope_points[i].pos);
    }

    DrawBoxAtV2d(sdl_state, camera, mouse_pos, 2, 2, false);

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
        DrawBoxAtV2d(sdl_state, camera, isct.intersection_point, 4, 4, false);
        DrawBoxAtV2d(sdl_state, camera, isct.projection_point, 4, 4, false);
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

    SdlStateInitialize(game.sdl_state, screenWidth, screenHeight);
    GameSpritesInitialize(game);
    GameDitherInitialize(game);
    // KidSpriteInitialize(game.kid_sprite, game.sdl_state.sdl_renderer);
    KidInitialize(game.kid);
    LevelInitialize(game.level, screenWidth, screenHeight);
    RopeStateInitialize(game.rs);
    TitleInitialize(game.title);

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(gameloop, (void*)&game, 0, 1);
    #else
    while (1) { gameloop((void*)&game); }
    #endif
}
