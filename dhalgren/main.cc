#include "include/sdl_state.h"
#include "include/v2d.h"
#include "include/kid.h"

#include <chrono>

static int kScreenWidth = 800;
static int kScreenHeight = 600;
static unsigned int kMillisecondsPerFrame = 1000/120;

struct Camera {
    V2d pos;
};

void CameraUpdate(Camera &camera, Kid &kid, KeyState &ks, V2d &mouse_pos, double dt) {
    V2d origin;
    V2d transform_screen = { (double)kScreenWidth / 2, (double)kScreenHeight / 2 };
    V2d ks_mp = { (double)ks.mx, (double)ks.my };

    camera.pos += (kid.pos - camera.pos) * dt;
    origin = camera.pos - transform_screen;

    mouse_pos = origin + ks_mp;
}

void DrawBoxAtV2d(SdlState &sdl_state, Camera &camera, V2d pos, int width, int height) {
    V2d transformed_pos;
    SDL_Rect sdl_rect;
    transformed_pos = pos - camera.pos;
    sdl_rect.x = transformed_pos.x + kScreenWidth / 2;
    sdl_rect.y = transformed_pos.y + kScreenHeight / 2;
    sdl_rect.w = width;
    sdl_rect.h = height;
    SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect);
}

void DrawAABB(SdlState &sdl_state, Camera &camera, AABB aabb) {
    DrawBoxAtV2d(sdl_state, camera, aabb.pos, aabb.width, aabb.height);
}

void DrawLine(SdlState &sdl_state, Camera &camera, V2d p1, V2d p2) {
    V2d transform_screen = { (double)kScreenWidth / 2, (double)kScreenHeight / 2 };
    p1 = p1 - camera.pos + transform_screen;
    p2 = p2 - camera.pos + transform_screen;
    SDL_RenderDrawLine(sdl_state.sdl_renderer, p1.x, p1.y, p2.x, p2.y);
}

// https://discourse.libsdl.org/t/poor-performance-of-sdl2-on-macos/28276/3
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

int main(int argc, char **argv) {
    bool exit = false;

    KeyState ks;

    SdlState sdl_state;

    SdlStateInitialize(sdl_state, kScreenWidth, kScreenHeight);

    unsigned int frame;
    double dt;
    double meters_per_pixel = 0.1;
    auto start_of_frame_clock = std::chrono::high_resolution_clock::now();
    auto end_of_update_clock = std::chrono::high_resolution_clock::now();
    SDL_TimerID timerID = SDL_AddTimer(kMillisecondsPerFrame, timerTickCallBack, NULL);

    V2d upward = { 0.0, 1.0 };

    V2d rightward = { 1.0, 0.0 };

    // ---

    Level level;

    Kid kid;

    KidInitialize(kid);

    KidUpdateContext kid_update_ctx;

    Camera camera;

    V2d mouse_pos;

    frame = 0;

    LevelInitialize(level, kScreenWidth, kScreenHeight);

    for (;!exit; ++frame) {
        start_of_frame_clock = std::chrono::high_resolution_clock::now();

        SdlStatePollEvents(ks, exit);

        dt = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-end_of_update_clock).count() / 1000000.0;
    
        kid_update_ctx.level = &level;
        kid_update_ctx.ks = &ks;
        kid_update_ctx.dt = dt;
        kid_update_ctx.meters_per_pixel = meters_per_pixel;
        KidUpdate(kid, kid_update_ctx);
        
        end_of_update_clock = std::chrono::high_resolution_clock::now();

        CameraUpdate(camera, kid, ks, mouse_pos, dt);

        LevelUpdate(level, ks, mouse_pos, dt);

        KeyStateClearPress(ks);

        // draw
        SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

        DrawLine(
            sdl_state,
            camera,
            kid.pos + upward * 4.0, // * (frame % 2),
            kid.pos - upward * 4.0 // * (frame % 2)
        );

        DrawLine(
            sdl_state,
            camera,
            kid.pos + rightward * 4.0, // * (frame % 2),
            kid.pos - rightward * 4.0 // * (frame % 2)
        );

        // DrawBoxAtV2d(sdl_state, camera, kid.pos, 16, 16);

        for (int i = 0; i < kSwingPoints - 1; ++i) {
            DrawBoxAtV2d(sdl_state, camera, kid.swing_pos[i], 2, 2);
            DrawLine(sdl_state, camera, kid.swing_pos[i], kid.swing_pos[i + 1]);
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

    return 0;
}