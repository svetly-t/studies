#include "include/sdl_state.h"
#include "include/v2d.h"
#include "include/kid.h"

int main(int argc, char **argv) {
    bool exit = false;

    KeyState ks;

    SdlState sdl_state;

    SdlStateInitialize(sdl_state, 800, 600);

    // ---

    Kid kid;

    kid.state = Kid::STAND;

    for (;!exit;) {
        SdlStatePollEvents(ks, exit);
    
        KidUpdate(kid, ks, 16.0/1000.0);

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

        SDL_RenderDrawRect(sdl_state.sdl_renderer, &sdl_rect1);

        SDL_UpdateWindowSurface(sdl_state.sdl_window);

        // sleep
        SDL_Delay(16);
    }

    return 0;
}