#pragma once

#include "v2d.h"

class Terrain {
 public:
    double Height(double pos) const;
    double Slope(double pos) const;
    V2d Tangent(double pos) const;
    V2d Normal(double pos) const;
};

