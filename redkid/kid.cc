#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

double Lerp(double from, double to, double factor) {
    return from + (to - from) * factor;
}

class V2d {
 public:
    V2d() {
        x = 0;
        y = 0;
    }
    V2d(double ix, double iy) {
        x = ix;
        y = iy;
    }
    double operator*(const V2d &other) {
        return x * other.x + y * other.y;
    }
    V2d& operator+=(const V2d& o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    V2d& operator-=(const V2d& o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }
    V2d& operator/=(const double& d) {
        x /= d;
        y /= d;
        return *this;
    }
    V2d& operator*=(const double& d) {
        x *= d;
        y *= d;
        return *this;
    }
    V2d operator*(const double& d) const {
        V2d result;
        result.x = x * d;
        result.y = y * d;
        return result;
    }
    V2d operator/(const double& d) const {
        V2d result;
        result.x = x / d;
        result.y = y / d;
        return result;
    }
    V2d operator+(const V2d& o) const {
        V2d result;
        result.x = x + o.x;
        result.y = y + o.y;
        return result;
    }
    V2d operator-(const V2d& o) const {
        V2d result;
        result.x = x - o.x;
        result.y = y - o.y;
        return result;
    }
    double operator*(const V2d& o) const {
        return x * o.x + y * o.y;
    }
    double Magnitude() {
        return x * x + y * y;
    }
    V2d Normalized() {
        return *this / this->Magnitude();
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
        FALLING,
    };
    State state;
    V2d pos;
    V2d vel;
    V2d acc;
};

class Camera {
 public:
    V2d pos;
};

const double kTerrainSlack = 0.1;

double terrain_function(double x) {
    return 2.0 * sin(x / 2.0);
}

double terrain_function_derivative(double x) {
    return cos(x / 2.0);
}

V2d terrain_function_tangent(double x) {
    V2d result;
    result.x = 1;
    result.y = terrain_function_derivative(x);
    return result.Normalized();
}

V2d terrain_function_normal(double x) {
    V2d result = terrain_function_tangent(x);
    double temp = result.x;
    result.x = -result.y;
    result.y = temp;
    return result;
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

    Camera camera;

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
    int kx = 0, ky = 0, kf = 0;
    int kxp = 0, kyp = 0, kfp = 0;
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
                            kx = -1;
                            if (!kx) kxp = -1;
                            break;
                        case SDLK_RIGHT:
                            kx = 1;
                            if (!kx) kxp = 1;
                            break;
                        case SDLK_UP:
                            ky = -1;
                            if (!ky) kyp = -1;
                            break;
                        case SDLK_DOWN:
                            ky = 1;
                            if (!ky) kyp = 1;
                            break;
                        case SDLK_SPACE:
                            kf = 1;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch( sdl_event.key.keysym.sym ){
                        case SDLK_LEFT:
                        case SDLK_RIGHT:
                            kx = 0;
                            break;
                        case SDLK_UP:
                        case SDLK_DOWN:
                            ky = 0;
                            break;
                        case SDLK_SPACE:
                            kf = 0;
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }

        double slope;
        double dot_product;
        V2d tangent = terrain_function_tangent(kid.pos.x);
        V2d normal = terrain_function_normal(kid.pos.x);
        V2d gravity(0.0, kGravity);
        V2d gravity_projected_onto_terrain =  tangent * (gravity * tangent);
        slope = terrain_function_derivative(kid.pos.x);
        dot_product = kGravity * slope;
        switch (kid.state) {
            // case Kid::WALKING:
            //     // Movement
            //     kid.vel.x = (double)kx;
            //     kid.vel.y = (double)kx * terrain_function_derivative(kid.pos.x);
            //     kid.pos.x += kid.vel.x * dt;
            //     kid.pos.y += kid.vel.y * dt;
            //     if (kf == 1) {
            //         kid.state = Kid::SLIDING;
            //         break;
            //     }
            //     break;
            case Kid::FALLING:
                kid.acc.x = 0;
                kid.acc.y = kGravity + (double)ky * kGravity;
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
                    kid.state = Kid::SLIDING;
                    break;
                }
                break;
            case Kid::SLIDING:
                // acceleration is (projection of weight onto tangent of terrain curve) / mass
                kid.acc = gravity_projected_onto_terrain;
                // Add to this the projection of directional boost
                kid.acc.x += (double)kx * 8.0;
                kid.acc.y += (double)kx * 8.0 * terrain_function_derivative(kid.pos.x);
                // kid.vel += a * dt
                kid.vel.x += kid.acc.x * dt;
                kid.vel.y += kid.acc.y * dt;
                // directly kick velocity on left, right button press
                if (kxp) {
                    kid.vel.x += (double)kx * 16.0;
                    kid.vel.y += (double)kx * 16.0 * terrain_function_derivative(kid.pos.x);
                }
                // kid.pos += kid.vel * dt
                kid.pos.x += kid.vel.x * dt;
                kid.pos.y += kid.vel.y * dt;
                // if significantly above the ground we're falling
                if (kid.pos.y < terrain_function(kid.pos.x) - kTerrainSlack) {
                    kid.state = Kid::FALLING;
                    break;
                }
                // constrain to keep the kid on the line
                if (kid.pos.y > terrain_function(kid.pos.x)) {
                    kid.pos.y = terrain_function(kid.pos.x);
                    // kill the portion of velocity that is in the direction of the terrain
                    kid.vel -= normal * (kid.vel * normal);
                }
                // Go back to walking if spacebar not pressed
                // if (kf == 0) {
                //     kid.state = Kid::WALKING;
                //     break;
                // }
                break;
            default:
                break;
        }

        // Transient button presses get cleared here
        kxp = 0;
        kyp = 0;
        kfp = 0;

        // Move the camera so that the player is always in the center of the view window
        // Add an offset so that, plus velocity vector, we shift in the direction player is going
        // camera.pos.x = kid.pos.x + kid.vel.x;
        camera.pos.x = Lerp(camera.pos.x, kid.pos.x + kid.vel.x, dt * 2.0);

        // camera.pos.y = kid.pos.y - (kWindowY / 2) * kPixelToDouble;

        // draw
        SDL_FillRect(sdl_surface, NULL, SDL_MapRGB(sdl_surface->format, 0, 0, 0));

        // kid struct to sdl native rect
        SDL_Rect sdl_rect1;
        sdl_rect1.x = (kid.pos.x - camera.pos.x) * kDoubleToPixel + kWindowX / 2;
        sdl_rect1.y = kid.pos.y * kDoubleToPixel + kWindowY / 2;
        sdl_rect1.w = kKidW;
        sdl_rect1.h = kKidH;
        
        SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);

        // draw terrain curve across the middle of screen
        const int kSegments = 100;
        int segment = 0;
        SDL_Point points[kSegments];
        // draw only the part of the curve that's in the window.
        double dx = camera.pos.x - (kWindowX / 2 * kPixelToDouble);
        for (; segment < kSegments; ++segment) {
            points[segment].x = kWindowX / kSegments * segment;
            points[segment].y = terrain_function(dx) * kDoubleToPixel + kWindowY / 2;

            dx += kWindowX / kSegments * kPixelToDouble;
        }
        SDL_RenderDrawLines(sdl_renderer, points, kSegments);

        // Draw debug velocity ray
        SDL_RenderDrawLine(sdl_renderer, sdl_rect1.x, sdl_rect1.y, sdl_rect1.x + kid.vel.x, sdl_rect1.y + kid.vel.y);

        // Draw kid rect
        SDL_RenderDrawRect(sdl_renderer, &sdl_rect1);

        SDL_UpdateWindowSurface(sdl_window);

        // sleep
        SDL_Delay(16);
    }

    return EXIT_SUCCESS;
}