#include "sdl_state.h"
#include "title.h"

void TitleSwitchState(Title &title, Title::State new_state) {
    title.state = new_state;
}

void TitleInitialize(Title &title) {
    title.state = Title::NOTHING;
    title.state_timer = 0;
}

void TitleUpdate(Title &title, KeyState &ks, double dt) {
    switch (title.state) {
        case Title::NOTHING:
            if (ks.spcp) {
                TitleSwitchState(title, Title::SELECTED);
                break;
            } 
            break;
        case Title::SELECTED:
            title.state_timer += dt;
            break;
    }
}