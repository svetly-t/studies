#pragma once

#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "v2d.h"
#include "image.h"
#include "terrain.h"
#include "puppet.h"

class Camera {
 public:
    const double kPixelToDouble = 0.1; // 1 pixel == 0.1 double; means kid is 1.6 meters
    const double kDoubleToPixel = 10; // 1 double == 10 pixels; means kid is 1.6 meters
    V2d pos;

    Camera(SDL_Renderer *sdl_renderer, int window_height, int window_width);

    void SetZoom(double pixel_to_double);

    double GetZoom();

    V2d ToScreenSpace(V2d at);

    V2d ToWorldSpace(V2d at);

    void DrawBox(V2d at);

    void DrawLeg(V2d at, V2d vel, Leg &leg);

    void DrawLine(V2d start, V2d end);

    void DrawCursor(V2d at);

    // draw terrain curve across the middle of screen
    void DrawTerrain(Terrain *terrain);

    void DrawAll(std::unordered_map<uint64_t, Line> &lines);

    void Draw(Line line);

 private:
    SDL_Renderer *_sdl_renderer;
    double _pixel_to_double;
    double _double_to_pixel;
    int _window_height;
    int _window_width;
};