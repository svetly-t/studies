#pragma once

#include "sdl_state.h"

struct KidSprite {
    SDL_Texture *active_sprite;
    int active_frame;
    int active_size;
    int active_vertical_index;

    double timer;
    double frame_time;
    int frame_count;

    SDL_Surface *sprite_sheet_surface;
    SDL_Texture *sprite_sheet_texture;
};

void KidSpriteInitialize(KidSprite &kid_sprite, SDL_Renderer *sdl_renderer);

void KidSpriteUpdate(KidSprite &kid_sprite, Kid &kid, double dt);
