#include "include/sdl_state.h"
#include "include/v2d.h"
#include "include/kid.h"

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

    // ---

    Level level;

    Kid kid;

    kid.state = Kid::STAND;

    ticks_after_update = SDL_GetTicks();

    for (;!exit;) {
        ticks_start_of_frame = SDL_GetTicks();

        SdlStatePollEvents(ks, exit);

        dt = (double)(SDL_GetTicks() - ticks_after_update) / 1000.0;
    
        KidUpdate(kid, level, ks, dt);

        LevelUpdate(level, ks, dt);

        ticks_after_update = SDL_GetTicks();

        KeyStateClearPress(ks);

        // draw
        SDL_FillRect(sdl_state.sdl_surface, NULL, SDL_MapRGB(sdl_state.sdl_surface->format, 0, 0, 0));

        SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 255, 255, 255);

        SDL_Rect sdl_rect1;
        sdl_rect1.x = kid.pos.x + 800 / 2;
        sdl_rect1.y = kid.pos.y + 600 / 2;
        sdl_rect1.w = 16;
        sdl_rect1.h = 16;
        // Draw kid rect
        SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect1);

        sdl_rect1.x = kid.swing_pos.x + 800 / 2;
        sdl_rect1.y = kid.swing_pos.y + 600 / 2;
        sdl_rect1.w = 2;
        sdl_rect1.h = 2;
        // Draw swing pos dot
        SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect1);

        sdl_rect1.x = ks.mx;
        sdl_rect1.y = ks.my;
        sdl_rect1.w = 2;
        sdl_rect1.h = 2;
        // Draw mouse rect
        SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect1);

        sdl_rect1.x = level.aabb.pos.x;
        sdl_rect1.y = level.aabb.pos.y;
        sdl_rect1.w = level.aabb.width;
        sdl_rect1.h = level.aabb.height;
        // Draw aabb rect
        SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect1);

        // Draw level test line
        SDL_RenderDrawLine(sdl_state.sdl_renderer, level.l1.x, level.l1.y, level.l2.x, level.l2.y);

        Line isct;

        if (AABBToLineIntersect(level.aabb, isct, level.l1, level.l2)) {
            // Draw level test line
            SDL_SetRenderDrawColor(sdl_state.sdl_renderer, 255, 0, 0, 255);
            SDL_RenderDrawLine(sdl_state.sdl_renderer, isct.p1.x, isct.p1.y, isct.p2.x, isct.p2.y);
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