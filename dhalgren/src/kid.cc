#include "kid.h"

void KidUpdate(Kid &kid, KeyState &ks, float dt) {
    switch (kid.state) {
        case Kid::STAND:
            if (ks.x != 0) {
                kid.state = Kid::START_RUN;
                kid.state_timer = 0.0;
                break;
            }
            break;
        case Kid::START_RUN:
            kid.state_timer += dt;
            if (kid.state_timer > 0.1) {
                kid.state = Kid::RUN;
                kid.state_timer = 0.0;
                break;
            }
            if (ks.x == 0) {
                kid.state = Kid::STAND;
                kid.state_timer = 0.0;
                break;
            }
            break;
        case Kid::RUN:
            kid.pos.x += ks.x * 5.0;
            if (ks.x == 0) {
                kid.state = Kid::STAND;
                kid.state_timer = 0.0;
                break;
            }
            break;
        default:
            break;
    }
}
