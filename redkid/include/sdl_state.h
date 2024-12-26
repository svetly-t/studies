#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

struct KeyState {
    // held
    int x = 0;
    int y = 0;
    int s = 0;
    int esc = 0;
    // pressed
    int xp = 0;
    int yp = 0;
    int sp = 0;
    int escp = 0;
    // mouse position relative to window
    int mx;
    int my;
    // mouse buttons held
    int mlc = 0;
    int mrc = 0;
    // mouse buttons pressed
    int mlcp = 0;
    int mrcp = 0;
};

class SdlState {
 public:
    SDL_Window *sdl_window;
    SDL_Surface *sdl_surface;
    SDL_Renderer *sdl_renderer;

    bool exit = false;

    SdlState(int window_width, int window_height) {
        SDL_Init(SDL_INIT_EVERYTHING);

        IMG_Init(IMG_INIT_PNG);
        // In theory this should be paired with an IMG_Quit() elsewhere

        sdl_window = SDL_CreateWindow("Newboy", 
                                      SDL_WINDOWPOS_UNDEFINED, 
                                      SDL_WINDOWPOS_UNDEFINED,
                                      window_width,
                                      window_height,
                                      SDL_WINDOW_ALLOW_HIGHDPI);
        if (!sdl_window) {
            abort();
        }

        sdl_surface = SDL_GetWindowSurface(sdl_window);
        if (!sdl_surface) {
            abort();
        }

        sdl_renderer = SDL_CreateSoftwareRenderer(sdl_surface);
    }

    void ClearPress(KeyState &ks) {
        ks.xp = 0;
        ks.yp = 0;
        ks.sp = 0;
        ks.escp = 0;
        ks.mlcp = 0;
        ks.mrcp = 0;
    }

    void GetEvents(KeyState &ks) {
        SDL_Event sdl_event;

        for (;SDL_PollEvent(&sdl_event) > 0;) {
            switch (sdl_event.type) {
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
                        case SDLK_SPACE:
                            ks.s = 1;
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
                        case SDLK_RIGHT:
                            ks.x = 0;
                            ks.xp = 0;
                            break;
                        case SDLK_UP:
                        case SDLK_DOWN:
                            ks.y = 0;
                            ks.yp = 0;
                            break;
                        case SDLK_SPACE:
                            ks.s = 0;
                            ks.sp = 0;
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
};