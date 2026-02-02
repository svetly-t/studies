#pragma once

#include "sdl_state.h"

struct KidSprite {
    SDL_Texture *active_sprite;
    int frame_idx;
    int vertical_idx;
    int size;

    double timer;
    double seconds_per_frame;
    int number_of_frames;

    SDL_Surface *sprite_sheet_surface;
    SDL_Texture *sprite_sheet_texture;
};

void KidSpriteInitialize(KidSprite &kid_sprite, SDL_Renderer *sdl_renderer);

void KidSpriteUpdate(KidSprite &kid_sprite, Kid &kid, double dt);
