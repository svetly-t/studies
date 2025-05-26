#include "include/sdl_state.h"
#include "include/v2d.h"
#include "include/kid.h"

void DrawBoxAtV2d(SdlState &sdl_state, V2d pos, int width, int height) {
    SDL_Rect sdl_rect;
    sdl_rect.x = pos.x;
    sdl_rect.y = pos.y;
    sdl_rect.w = width;
    sdl_rect.h = height;
    SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect);
}

void DrawAABB(SdlState &sdl_state, AABB aabb) {
    DrawBoxAtV2d(sdl_state, aabb.pos, aabb.width, aabb.height);
}

void DrawLine(SdlState &sdl_state, V2d p1, V2d p2) {
    SDL_RenderDrawLine(sdl_state.sdl_renderer, p1.x, p1.y, p2.x, p2.y);
}

int main(int argc, char **argv) {
    bool exit = false;

    KeyState ks;

    SdlState sdl_state;

    SdlStateInitialize(sdl_state, 800, 600);

    unsigned int ticks_start_of_frame;
    unsigned int ticks_after_update;
    unsigned int ticks_in_frame;
    unsigned int ticks_to_next_frame;
    double dt;
    double meters_per_pixel = 0.1;

    // ---

    Level level;

    Kid kid;

    kid.state = Kid::STAND;

    KidUpdateContext kid_update_ctx;

    ticks_after_update = SDL_GetTicks();

    LevelInitialize(level, 800, 600);

    for (;!exit;) {
        ticks_start_of_frame = SDL_GetTicks();

        SdlStatePollEvents(ks, exit);

        dt = (double)(SDL_GetTicks() - ticks_after_update) / 1000.0;
    
        kid_update_ctx.level = &level;
        kid_update_ctx.ks = &ks;
        kid_update_ctx.dt = dt;
        kid_update_ctx.meters_per_pixel = meters_per_pixel;
        KidUpdate(kid, kid_update_ctx);

        LevelUpdate(level, ks, dt);

        ticks_after_update = SDL_GetTicks();

        KeyStateClearPress(ks);

        // draw
        SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

        DrawBoxAtV2d(sdl_state, kid.pos, 16, 16);

        DrawBoxAtV2d(sdl_state, kid.swing_pos, 2, 2);

        DrawBoxAtV2d(sdl_state, { (double)ks.mx, (double)ks.my }, 2, 2);

        DrawAABB(sdl_state, level.aabb);

        for (auto &aabb : level.aabbs) {
            DrawAABB(sdl_state, aabb);
        }

        // Draw level test line
        DrawLine(sdl_state, level.l1, level.l2);

        // Draw kid velocity line
        DrawLine(sdl_state, kid.pos, kid.pos + kid.vel);

        LineToLineIntersection isct = AABBToLineIntersect(level.aabb, level.l1, level.l2);
        if (isct.exists) {
            // Draw level test line
            SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 0, 0, 255);
            DrawBoxAtV2d(sdl_state, isct.intersection_point, 4, 4);
            DrawBoxAtV2d(sdl_state, isct.projection_point, 4, 4);
            DrawLine(sdl_state, isct.projection_point, isct.projection_point + isct.normal * 8.0);
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