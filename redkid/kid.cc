#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

class V2d {
 public:
    V2d() {
        x = 0;
        y = 0;
    }
    double x;
    double y;
};

class Kid {
 public:
    Kid() {
        state = State::SLIDING;
    }
    enum State {
        WALKING,
        SLIDING,
    };
    State state;
    V2d pos;
    V2d vel;
    V2d acc;
};

double terrain_function(double x) {
    return 2.0 * sin(x / 2.0);
}

double terrain_function_derivative(double x) {
    return cos(x / 2.0);
}

int main(int argc, char **argv) {
    const int kWindowX = 800; 
    const int kWindowY = 600;
    const int kKidW = 16;
    const int kKidH = 16;
    const double kPixelToDouble = 0.1; // 1 pixel == 0.1 double; means kid is 1.6 meters
    const double kDoubleToPixel = 10; // 1 double == 10 pixels; means kid is 1.6 meters
    const double kGravity = 20.0;

    double kKidStartX = 8;
    double kKidStartY = terrain_function(kKidStartX);

    Kid kid;
    kid.pos.x = kKidStartX;
    kid.pos.y = kKidStartY;

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

    // load kid.png sprite sheet

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

        double slope;
        double dot_product;
        slope = terrain_function_derivative(kid.pos.x);
        dot_product = kGravity * slope;
        switch (kid.state) {
            case Kid::SLIDING:
                // use position to find point on curve
                // get projection of gravity onto slope of curve
                // (dot product is <1, mg> * <1, terrain_func_dx>)
                // (pretend mass is 1 or something, just ignore)
                // acceleration is a = (projection of weight onto tangent of curve) / mass
                kid.acc.x = dot_product;
                kid.acc.y = dot_product * slope;
                // kid.vel += a * dt
                kid.vel.x += kid.acc.x * dt;
                kid.vel.y += kid.acc.y * dt;
                // kid.pos += kid.vel * dt
                kid.pos.x += kid.vel.x * dt;
                kid.pos.y += kid.vel.y * dt;
                // constrain to keep the kid on the line
                if (kid.pos.y > terrain_function(kid.pos.x)) {
                    kid.pos.y = terrain_function(kid.pos.x);
                    kid.vel.y -= kid.acc.y * dt;
                }
                break;
            default:
                break;
        }

        // draw
        SDL_FillRect(sdl_surface, NULL, SDL_MapRGB(sdl_surface->format, 0, 0, 0));

        // kid struct to sdl native rect
        SDL_Rect sdl_rect1;
        sdl_rect1.x = kid.pos.x * kDoubleToPixel + kWindowX / 2;
        sdl_rect1.y = kid.pos.y * kDoubleToPixel + kWindowY / 2;
        sdl_rect1.w = kKidW;
        sdl_rect1.h = kKidH;
        
        SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

        // draw terrain curve across the middle of screen
        const int kSegments = 100;
        int segment = 0;
        SDL_Point points[kSegments];
        double dx = -kWindowX / 2 * kPixelToDouble;
        double dy;
        for (; segment < kSegments; ++segment) {
            points[segment].x = kWindowX / kSegments * segment;
            points[segment].y = terrain_function(dx) * kDoubleToPixel + kWindowY / 2;

            dx += kWindowX / kSegments * kPixelToDouble;
        }
        SDL_RenderDrawLines(sdl_renderer, points, kSegments);

        // Draw kid rect
        SDL_RenderDrawRect(sdl_renderer, &sdl_rect1);

        SDL_UpdateWindowSurface(sdl_window);

        // sleep
        SDL_Delay(16);
    }

    return EXIT_SUCCESS;
}