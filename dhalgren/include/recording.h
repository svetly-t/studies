#pragma once

#include "sdl_state.h"
#include "kid.h"

struct SDL_Event_List_Node {
    SDL_Event_List_Node *next;
    SDL_Event event;
};

struct SDLStatePollNode {
    SDLStatePollNode *next;
    SDL_Event_List_Node *events;
};

struct Recording {
    enum State {
        RECORDING,
        REPLAYING,
        NOTHING,
    };
    State state;
    Kid kid;
    SDLStatePollNode head;
};

void RecordingAddEvent(Recording &recording, SDL_Event &event);
