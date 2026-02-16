#include "kid.h"
#include "kidsprite.h"

void KidSpriteInitialize(KidSprite &kid_sprite, SDL_Renderer *sdl_renderer) {
    SdlSpriteLoad(kid_sprite.sprite_sheet_surface, kid_sprite.sprite_sheet_texture, sdl_renderer, "./img/sprite_sheet_3.png");
    kid_sprite.size_src = 500;
    kid_sprite.size_dst = 16;
    kid_sprite.frame_idx = 0;
    kid_sprite.vertical_idx = 0;
    kid_sprite.active_sprite = kid_sprite.sprite_sheet_texture;
    kid_sprite.timer = 0;
}

void KidSpriteUpdate(KidSprite &kid_sprite, Kid &kid, double dt) {
    kid_sprite.timer += dt;
    if (kid_sprite.timer > 1.0)
        kid_sprite.timer = 0.0;

    switch (kid.state) {
        case Kid::STAND:
            kid_sprite.number_of_frames = 2;
            kid_sprite.seconds_per_frame = 0.25;
            kid_sprite.vertical_idx = 0;
            break;
        case Kid::CHARGE_RUN:
            kid_sprite.number_of_frames = 2;
            kid_sprite.seconds_per_frame = 0.5 / (kid.state_timer + 1.0);
            kid_sprite.vertical_idx = 0;
            break;
        case Kid::RUN:
            kid_sprite.number_of_frames = 2;
            kid_sprite.seconds_per_frame = 0.3 / (kid.vel.x + 1.0);
            kid_sprite.vertical_idx = 0;
            break;
        case Kid::JUMP:
            kid_sprite.number_of_frames = 1;
            kid_sprite.seconds_per_frame = 0.3 / (kid.vel.x + 1.0);
            kid_sprite.vertical_idx = 1;
            break;
        case Kid::SWING:
            kid_sprite.number_of_frames = 1;
            kid_sprite.seconds_per_frame = 0.3 / (kid.vel.x + 1.0);
            kid_sprite.vertical_idx = 3;
            break;
        default:
            kid_sprite.number_of_frames = 2;
            kid_sprite.seconds_per_frame = 0.5;
            kid_sprite.vertical_idx = 2;
            break;
    }

    kid_sprite.frame_idx = (int)(kid_sprite.timer / kid_sprite.seconds_per_frame) % kid_sprite.number_of_frames;
}
