#pragma once

#include "v2d.h"
#include "terrain.h"

class Camera {
 public:
    const double kPixelToDouble = 0.1; // 1 pixel == 0.1 double; means kid is 1.6 meters
    const double kDoubleToPixel = 10; // 1 double == 10 pixels; means kid is 1.6 meters
    V2d pos;

    Camera(SDL_Renderer *sdl_renderer, int window_height, int window_width) {
        _sdl_renderer = sdl_renderer;
        _window_height = window_height;
        _window_width = window_width;
        _pixel_to_double = kPixelToDouble;
        _double_to_pixel = kDoubleToPixel;
    }

    V2d ToScreenSpace(V2d at) {
        V2d result;
        result.x = (at.x - pos.x) * _double_to_pixel + _window_width / 2;
        result.y = at.y * _double_to_pixel + _window_height / 2;
        return result;  
    }

    void DrawBox(V2d at) {
        SDL_Rect rect;
        rect.x = (at.x - pos.x) * _double_to_pixel + _window_width / 2;
        rect.y = at.y * _double_to_pixel + _window_height / 2;
        rect.w = 1.6 * _double_to_pixel;
        rect.h = 1.6 * _double_to_pixel;

        SDL_RenderDrawRect(_sdl_renderer, &rect);
    }

    // draw terrain curve across the middle of screen
    void DrawTerrain(Terrain &terrain) {
        const int kSegments = 100;
        int segment = 0;
        double dx;
        SDL_Point points[kSegments];
        
        // draw only the part of the curve that's in the window.
        dx = pos.x - (_window_width / 2 * _pixel_to_double);
        for (; segment < kSegments; ++segment) {
            points[segment].x = _window_width / kSegments * segment;
            points[segment].y = terrain.Height(dx) * _double_to_pixel + _window_height / 2;

            dx += _window_width / kSegments * _pixel_to_double;
        }
        SDL_RenderDrawLines(_sdl_renderer, points, kSegments);
    }

 private:
    SDL_Renderer *_sdl_renderer;
    double _pixel_to_double;
    double _double_to_pixel;
    int _window_height;
    int _window_width;
};