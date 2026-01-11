#include "sdl_state.h"

#ifdef __EMSCRIPTEN__
#include <SDL_hints.h>
#else
#include <SDL2/SDL_hints.h>
#endif

void SdlStateInitialize(SdlState &sdl_state, int window_width, int window_height) {
    SDL_Init(SDL_INIT_EVERYTHING);

    IMG_Init(IMG_INIT_PNG);

    sdl_state.sdl_window = SDL_CreateWindow("Newboy", 
                                  SDL_WINDOWPOS_UNDEFINED, 
                                  SDL_WINDOWPOS_UNDEFINED,
                                  window_width,
                                  window_height,
                                  0);
    if (!sdl_state.sdl_window) {
        abort();
    }

    // SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    sdl_state.sdl_surface = SDL_GetWindowSurface(sdl_state.sdl_window);
    if (!sdl_state.sdl_surface) {
        abort();
    }

    sdl_state.sdl_renderer = SDL_CreateSoftwareRenderer(sdl_state.sdl_surface);
}

void SdlStatePollEvents(KeyState &ks, bool &exit) {
    SDL_Event sdl_event;

#ifdef __EMSCRIPTEN__
    while (SDL_PollEvent(&sdl_event))
#else
    while (SDL_WaitEvent(&sdl_event))
#endif
    {
        switch (sdl_event.type) {
            case SDL_USEREVENT:
                // This is the frame-time event.
                // If we see this, we need to proceed with the game logic, so break out with 'return'
                return;
            case SDL_QUIT:
                exit = true;
                break;
            case SDL_KEYDOWN:
                switch( sdl_event.key.keysym.sym ){
                    case SDLK_LEFT:
                        if (!ks.x) ks.xp = -1;
                        ks.x = -1;
                        break;
                    case SDLK_RIGHT:
                        if (!ks.x) ks.xp = 1;
                        ks.x = 1;
                        break;
                    case SDLK_UP:
                        if (!ks.y) ks.yp = -1;
                        ks.y = -1;
                        break;
                    case SDLK_DOWN:
                        if (!ks.y) ks.yp = 1;
                        ks.y = 1;
                        break;
                    case SDLK_s:
                        if (!ks.s) ks.sp = 1;
                        ks.s = 1;
                        break;
                    case SDLK_l:
                        if (!ks.l) ks.lp = 1;
                        ks.l = 1;
                    case SDLK_r:
                        if (!ks.r) ks.rp = 1;
                        ks.r = 1;
                        break;
                    case SDLK_e:
                    case SDLK_KP_E:
                        if (!ks.e) ks.ep = 1;
                        ks.e = 1;
                        break;
                    case SDLK_c:
                    case SDLK_KP_C:
                        if (!ks.c) ks.cp = 1;
                        ks.c = 1;
                        break;
                    case SDLK_z:
                        if (!ks.z) ks.zp = 1;
                        ks.z = 1;
                        break;
                    case SDLK_SPACE:
                        if (!ks.spc) ks.spcp = 1;
                        ks.spc = 1;
                        break;
                    case SDLK_ESCAPE:
                        if (!ks.esc) ks.escp = 1;
                        ks.esc = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_KEYUP:
                switch( sdl_event.key.keysym.sym ){
                    case SDLK_LEFT:
                        if (ks.x == -1) {
                            ks.x = 0;
                            ks.xp = 0;
                            break;
                        }
                    case SDLK_RIGHT:
                        if (ks.x == 1) {
                            ks.x = 0;
                            ks.xp = 0;
                            break;
                        }
                    case SDLK_UP:
                    case SDLK_DOWN:
                        ks.y = 0;
                        ks.yp = 0;
                        break;
                    case SDLK_SPACE:
                        ks.spc = 0;
                        ks.spcp = 0;
                        break;
                    case SDLK_s:
                        ks.s = 0;
                        ks.sp = 0;
                        break;
                    case SDLK_l:
                        ks.l = 0;
                        ks.lp = 0;
                        break;
                    case SDLK_r:
                        ks.r = 0;
                        ks.rp = 0;
                        break;
                    case SDLK_e:
                    case SDLK_KP_E:
                        ks.e = 0;
                        ks.ep = 0;
                        break;
                    case SDLK_c:
                    case SDLK_KP_C:
                        ks.c = 0;
                        ks.cp = 0;
                        break;
                    case SDLK_z:
                        ks.z = 0;
                        ks.zp = 0;
                        break;
                    case SDLK_ESCAPE:
                        ks.esc = 0;
                        ks.escp = 0;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                ks.mx = sdl_event.motion.x;
                ks.my = sdl_event.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch ( sdl_event.button.button ){
                    case 1:
                        if (!ks.mlc) ks.mlcp = 1;
                        ks.mlc = 1;
                        break;
                    case 3:
                        if (!ks.mrc) ks.mrcp = 1;
                        ks.mrc = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch ( sdl_event.button.button ){
                    case 1:
                        ks.mlc = 0;
                        ks.mlcp = 0;
                        break;
                    case 3:
                        ks.mrc = 0;
                        ks.mrcp = 0;
                        break;
                    default:
                        break;
                }
                break;
        }
    }
}

void KeyStateClearPress(KeyState &ks) {
    ks.xp = 0;
    ks.yp = 0;
    ks.ep = 0;
    ks.cp = 0;
    ks.zp = 0;
    ks.sp = 0;
    ks.lp = 0;
    ks.spcp = 0;
    ks.escp = 0;
    ks.mlcp = 0;
    ks.mrcp = 0;
}
