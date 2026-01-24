#include "sdl_state.h"
#include "title.h"

void TitleSwitchState(Title &title, Title::State new_state) {
    title.state = new_state;
    title.state_timer = 0.0;
}

void TitleInitialize(Title &title) {
    title.state = Title::NOTHING;
    title.state_timer = 0;
}

void TitleUpdate(Title &title, KeyState &ks, double dt) {
    switch (title.state) {
        case Title::HIDDEN:
            if (ks.spcp) {
                TitleSwitchState(title, Title::SELECTED);
                break;
            }
            if (title.state_timer > 0.75) {
                TitleSwitchState(title, Title::NOTHING);
                break;
            }
            title.state_timer += dt;
        case Title::NOTHING:
            if (ks.spcp) {
                TitleSwitchState(title, Title::SELECTED);
                break;
            }
            title.state_timer += dt;
            break;
        case Title::SELECTED:
            title.state_timer += dt;
            break;
    }
}