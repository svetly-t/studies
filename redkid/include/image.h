#pragma once

#include <cstdint>
#include <vector>

#include "v2d.h"

class Drawable {
 public:
    bool is_screenspace;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class Box : public Drawable {
 public:
    V2d pos;
    V2d dim;
};

class Line : public Drawable {
 public:
    std::vector<V2d> points;
};
