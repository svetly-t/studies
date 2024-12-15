#include <cmath>

#include "v2d.h"
#include "terrain.h"

double DefaultTerrain::Height(double x) const {
    return 2.0 * sin(x / 2.0);
}

double DefaultTerrain::Slope(double x) const {
    return cos(x / 2.0);
}

V2d DefaultTerrain::Tangent(double x) const {
    V2d result;
    result.x = 1;
    result.y = Slope(x);
    return result.Normalized();
}

V2d DefaultTerrain::Normal(double x) const {
    V2d result = Tangent(x);
    double temp = result.x;
    result.x = -result.y;
    result.y = temp;
    return result;
}
