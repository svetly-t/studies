#include "include/sdl_state.h"
#include "include/v2d.h"
#include "include/kid.h"

static int kScreenWidth = 800;
static int kScreenHeight = 600;

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

int main(int argc, char **argv) {
    bool exit = false;

    KeyState ks;

    SdlState sdl_state;

    SdlStateInitialize(sdl_state, kScreenWidth, kScreenHeight);

    unsigned int frame;
    unsigned int ticks_start_of_frame;
    unsigned int ticks_after_update;
    unsigned int ticks_in_frame;
    unsigned int ticks_to_next_frame;
    double dt;
    double meters_per_pixel = 0.1;

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

    ticks_after_update = SDL_GetTicks();

    LevelInitialize(level, kScreenWidth, kScreenHeight);

    for (;!exit; ++frame) {
        ticks_start_of_frame = SDL_GetTicks();

        SdlStatePollEvents(ks, exit);

        dt = (double)(SDL_GetTicks() - ticks_after_update) / 1000.0;
    
        kid_update_ctx.level = &level;
        kid_update_ctx.ks = &ks;
        kid_update_ctx.dt = dt;
        kid_update_ctx.meters_per_pixel = meters_per_pixel;
        KidUpdate(kid, kid_update_ctx);

        CameraUpdate(camera, kid, ks, mouse_pos, dt);

        LevelUpdate(level, ks, mouse_pos, dt);

        ticks_after_update = SDL_GetTicks();

        KeyStateClearPress(ks);

        // draw
        SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

        DrawLine(
            sdl_state,
            camera,
            kid.pos + upward * (frame % 2) * 4.0,
            kid.pos - upward * (frame % 2) * 4.0
        );

        DrawLine(
            sdl_state,
            camera,
            kid.pos + rightward * (frame % 2) * 4.0,
            kid.pos - rightward * (frame % 2) * 4.0
        );

        // DrawBoxAtV2d(sdl_state, camera, kid.pos, 16, 16);

        for (int i = 0; i < kSwingPoints - 2; ++i) {
            DrawBoxAtV2d(sdl_state, camera, kid.swing_pos[0], 2, 2);
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

        // How long did the frame take?
        ticks_in_frame = SDL_GetTicks() - ticks_start_of_frame;

        ticks_to_next_frame = 16 - ticks_in_frame;
        if (ticks_in_frame > 16)
            ticks_to_next_frame = 0;

        // sleep
        SDL_Delay(ticks_to_next_frame);
    }

    return 0;
}