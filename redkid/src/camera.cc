#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "v2d.h"
#include "utilities.h"
#include "terrain.h"
#include "camera.h"

Camera::Camera(SDL_Renderer *sdl_renderer, int window_height, int window_width) {
    _sdl_renderer = sdl_renderer;
    _window_height = window_height;
    _window_width = window_width;
    _pixel_to_double = kPixelToDouble;
    _double_to_pixel = kDoubleToPixel;
}

void Camera::SetZoom(double pixel_to_double) {
    _pixel_to_double = pixel_to_double;
    _double_to_pixel = 1.0 / pixel_to_double;
}

double Camera::GetZoom() {
    return _pixel_to_double;
}

V2d Camera::ToScreenSpace(V2d at) {
    V2d result;
    result.x = (at.x - pos.x) * _double_to_pixel + _window_width / 2;
    result.y = (at.y - pos.y) * _double_to_pixel + _window_height / 2;
    return result;  
}

V2d Camera::ToWorldSpace(V2d at) {
    V2d result;
    at.x -=  _window_width / 2;
    at.y -=  _window_height / 2;
    result.x = at.x * _pixel_to_double + pos.x;
    result.y = at.y * _pixel_to_double + pos.y;
    return result;  
}

void Camera::DrawBox(V2d at) {
    SDL_Rect rect;
    at.x -= 0.8;
    at.y -= 1.6;
    rect.x = (at.x - pos.x) * _double_to_pixel + _window_width / 2;
    rect.y = (at.y - pos.y) * _double_to_pixel + _window_height / 2;
    rect.w = 1.6 * _double_to_pixel;
    rect.h = 1.6 * _double_to_pixel;

    SDL_RenderDrawRect(_sdl_renderer, &rect);
}

void Camera::DrawSprite(V2d at, SDL_Texture *texture, int src_size, int src_frame, int dst_size, int dst_offset) {
    SDL_Rect src;
    SDL_Rect dst;
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    
    at = ToScreenSpace(at);

    src.y = 0;
    src.x = src_size * src_frame;
    src.w = src_size;
    src.h = src_size;

    if (dst_size < 0) {
        flip = SDL_FLIP_HORIZONTAL;
        dst_size = -dst_size;
    }

    dst.x = at.x - dst_size / 2;
    dst.y = at.y - dst_size / 2 - dst_offset; 
    dst.w = dst_size;
    dst.h = dst_size;

    SDL_RenderCopyEx(_sdl_renderer, texture, &src, &dst, 0, nullptr, flip);
}

void Camera::DrawLeg(V2d at, V2d vel, Leg &leg) {
    V2d upper_leg_end;
    V2d lower_leg_end;
    V2d foot_end;

    double radian_1 = degToRad(leg.theta_1);
    double radian_2 = radian_1 - degToRad(180.0 - leg.theta_2);

    at = ToScreenSpace(at);

    upper_leg_end = at + V2d(-std::cos(radian_1), std::sin(radian_1)) * 16.0;
    lower_leg_end = upper_leg_end + V2d(-std::cos(radian_2), std::sin(radian_2)) * 16.0;

    DrawLine(at, upper_leg_end);
    DrawLine(upper_leg_end, lower_leg_end);
}

void Camera::DrawLine(V2d start, V2d end) {
    SDL_RenderDrawLine(_sdl_renderer, start.x, start.y, end.x, end.y);
}

void Camera::DrawCursor(V2d at) {
    SDL_RenderDrawLine(_sdl_renderer, at.x - 3, at.y, at.x + 3, at.y);
    SDL_RenderDrawLine(_sdl_renderer, at.x, at.y - 3, at.x, at.y + 3);
}

// draw terrain curve across the middle of screen
void Camera::DrawTerrain(Terrain *terrain) {
    const int kSegments = 100;
    int segment = 0;
    double dx;
    V2d point;
    SDL_Point points[kSegments];

    if (terrain == nullptr)
        return;
    
    // draw only the part of the curve that's in the window.
    dx = pos.x - (_window_width / 2 * _pixel_to_double);
    for (; segment < kSegments; ++segment) {
        point.x = dx;
        point.y = terrain->RawHeight(point.x);
        point = ToScreenSpace(point);
        points[segment].y = point.y;
        points[segment].x = point.x;

        dx += _window_width / kSegments * _pixel_to_double;
    }
    SDL_RenderDrawLines(_sdl_renderer, points, kSegments);
}
