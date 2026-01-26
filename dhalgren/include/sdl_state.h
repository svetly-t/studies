#pragma once

#ifdef __EMSCRIPTEN__
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

struct KeyState {
    // held
    int x = 0;
    int y = 0;
    int s = 0;
    int l = 0;
    int r = 0;
    int spc = 0;
    int e = 0;
    int c = 0;
    int z = 0;
    int esc = 0;
    // pressed
    int xp = 0;
    int yp = 0;
    int sp = 0;
    int lp = 0;
    int rp = 0;
    int spcp = 0;
    int cp = 0;
    int ep = 0;
    int zp = 0;
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

struct SdlState {
    SDL_Window *sdl_window;
    SDL_Surface *sdl_surface;
    SDL_Renderer *sdl_renderer;

    bool exit = false;
};

void SdlSpriteLoad(SDL_Surface *&sprite_surface, SDL_Texture *&sprite_texture, SDL_Renderer *sdl_renderer, const char *path);

void SdlStateInitialize(SdlState &sdl_state, int window_width, int window_height);

void SdlStatePollEvents(KeyState &ks, bool &exit);

void KeyStateClearPress(KeyState &ks);
