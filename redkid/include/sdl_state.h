#pragma once

class KeyState {
 public:
    // held
    int x = 0;
    int y = 0;
    int s = 0;
    // pressed
    int xp = 0;
    int yp = 0;
    int sp = 0;
};

class SdlState {
 public:
    SDL_Window *sdl_window;
    SDL_Surface *sdl_surface;
    SDL_Renderer *sdl_renderer;

    bool exit = false;

    SdlState(int window_width, int window_height) {
        SDL_Init(SDL_INIT_EVERYTHING);

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
                            ks.x = -1;
                            if (!ks.x) ks.xp = -1;
                            break;
                        case SDLK_RIGHT:
                            ks.x = 1;
                            if (!ks.x) ks.xp = 1;
                            break;
                        case SDLK_UP:
                            ks.y = -1;
                            if (!ks.y) ks.yp = -1;
                            break;
                        case SDLK_DOWN:
                            ks.y = 1;
                            if (!ks.y) ks.yp = 1;
                            break;
                        case SDLK_SPACE:
                            ks.s = 1;
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
                            break;
                        case SDLK_UP:
                        case SDLK_DOWN:
                            ks.y = 0;
                            break;
                        case SDLK_SPACE:
                            ks.s = 0;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }
    }
};