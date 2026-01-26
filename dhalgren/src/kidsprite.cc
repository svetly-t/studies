#include "kid.h"
#include "kidsprite.h"

void KidSpriteInitialize(KidSprite &kid_sprite, SDL_Renderer *sdl_renderer) {
    SdlSpriteLoad(kid_sprite.sprite_sheet_surface, kid_sprite.sprite_sheet_texture, sdl_renderer, "./img/alternating-colors.png");
    kid_sprite.active_size = 16;
    kid_sprite.active_frame = 0;
    kid_sprite.active_vertical_index = 0;
    kid_sprite.active_sprite = kid_sprite.sprite_sheet_texture;
    kid_sprite.timer = 0;
}

void KidSpriteUpdate(KidSprite &kid_sprite, Kid &kid, double dt) {
    kid_sprite.timer += dt;
    if (kid_sprite.timer > 1.0)
        kid_sprite.timer = 0.0;

    switch (kid.state) {
        case Kid::STAND:
            kid_sprite.frame_count = 2;
            kid_sprite.frame_time = 0.25;
            kid_sprite.active_vertical_index = 0;
            break;
        case Kid::CHARGE_RUN:
            kid_sprite.frame_count = 2;
            kid_sprite.frame_time = 0.5 / (kid.state_timer + 1.0);
            kid_sprite.active_vertical_index = 1;
            break;
        case Kid::RUN:
            kid_sprite.frame_count = 2;
            kid_sprite.frame_time = 0.3 / (kid.vel.x + 1.0);
            kid_sprite.active_vertical_index = 1;
            break;
        default:
            kid_sprite.frame_count = 2;
            kid_sprite.frame_time = 0.5;
            kid_sprite.active_vertical_index = 2;
            break;
    }

    kid_sprite.active_frame = (int)(kid_sprite.timer / kid_sprite.frame_time) % kid_sprite.frame_count;
}
