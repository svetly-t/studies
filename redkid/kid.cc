#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

class Kid {
 public:
    Kid(double init_x, double init_y) {
        x = init_x;
        y = init_y;
        state = State::GROUND;
    }
    enum State {
        GROUND,
        START_JUMPING,
        JUMPING
    };
    State state;
    double x;
    double y;
    double y_vel;
    double x_vel;
};

int main(int argc, char **argv) {
    const int kWindowX = 800; 
    const int kWindowY = 600;

    Kid kid(kWindowX / 2, kWindowY / 2);

    SDL_Init(SDL_INIT_EVERYTHING);
    
    SDL_Window *sdl_window = SDL_CreateWindow("Newboy", 
                                          SDL_WINDOWPOS_UNDEFINED, 
                                          SDL_WINDOWPOS_UNDEFINED,
                                          kWindowX,
                                          kWindowY,
                                          SDL_WINDOW_ALLOW_HIGHDPI);
    if (!sdl_window) {
        std::cout << "Could not create window " << SDL_GetError() << std::endl;
        abort();
    }

    SDL_Surface *sdl_surface = SDL_GetWindowSurface(sdl_window);
    if (!sdl_surface) {
        std::cout << "No surface " << SDL_GetError() << std::endl;
        abort();
    }

    SDL_Renderer *sdl_renderer = SDL_CreateSoftwareRenderer(sdl_surface);

    SDL_KeyboardEvent *key;

    SDL_Event sdl_event;
    bool exit = false;
    int x = 0, y = 0;
    float dt = 16.0 / 1000.0;
    for (;!exit;) {
        // Event loop
        for (;SDL_PollEvent(&sdl_event) > 0;) {
            switch (sdl_event.type) {
                case SDL_QUIT:
                    exit = true;
                    break;
                case SDL_KEYDOWN:
                    switch( sdl_event.key.keysym.sym ){
                        case SDLK_LEFT:
                            x = -1;
                            break;
                        case SDLK_RIGHT:
                            x = 1;
                            break;
                        case SDLK_UP:
                            y = -1;
                            break;
                        case SDLK_DOWN:
                            y = 1;
                            break;
                        default:
                            break;
                    }
                break;
                case SDL_KEYUP:
                    switch( sdl_event.key.keysym.sym ){
                        case SDLK_LEFT:
                        case SDLK_RIGHT:
                            x = 0;
                            break;
                        case SDLK_UP:
                        case SDLK_DOWN:
                            y = 0;
                            break;
                        default:
                            break;
                    }
                break;
            }
        }

        switch (kid.state) {
            case Kid::GROUND:
                kid.x += (double)x * 20 * dt;
                if (y == -1)
                    kid.state = Kid::START_JUMPING;
                break;
            case Kid::START_JUMPING:
                kid.y_vel = 100;
                kid.x_vel = (double)x * 20 * dt;
                kid.state = Kid::JUMPING;
                break;
            case Kid::JUMPING:
                kid.y -= kid.y_vel * dt;
                kid.y_vel -= 100.0 * dt;
                kid.x += kid.x_vel * 20 * dt;
                if (kid.y >= kWindowY / 2) {
                    kid.y = kWindowY / 2;
                    kid.state = Kid::GROUND;
                }
                break;
            default:
                break;
        }

        // draw
        SDL_FillRect(sdl_surface, NULL, SDL_MapRGB(sdl_surface->format, 0, 0, 0));

        // kid struct to sdl native rect
        SDL_Rect sdl_rect1;
        sdl_rect1.x = kid.x;
        sdl_rect1.y = kid.y;
        sdl_rect1.w = 16;
        sdl_rect1.h = 16;
        
        SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(sdl_renderer, &sdl_rect1);

        SDL_UpdateWindowSurface(sdl_window);

        // sleep
        SDL_Delay(16);
    }

    return EXIT_SUCCESS;

    // if left, decrement x
    // if right, increment x

    // if spacebar, jump
}