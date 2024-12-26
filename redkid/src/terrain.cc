#include <cmath>

#include "v2d.h"
#include "terrain.h"
#include "lerp.h"

double DefaultTerrain::Height(double x) const {
    return 2.0 * sin(x / 2.0);
}

double DefaultTerrain::RawHeight(double &x) const {
    return Height(x);
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
    return -result;
}

void BuiltTerrain::Initialize(size_t number_of_points, double double_between_points) {
    _number_of_points = number_of_points;
    _double_between_points = double_between_points;
    _points.assign(number_of_points, 0.0);
}

void BuiltTerrain::getIndices(double x, size_t &floor, size_t &ceil) const {
    double high_idx;
    double low_idx = (_number_of_points * _double_between_points) / 2.0 + x;
    // Lock index to range of vector
    if (low_idx < 0.0) {
        floor = 0;
        ceil = 0;
        return;
    }
    if (low_idx > _number_of_points) {
        floor = _number_of_points - 1;
        ceil = _number_of_points - 1;
        return;
    }
    floor = (size_t)(low_idx);
    ceil = floor + 1;
}

void BuiltTerrain::SetHeight(double x, double y) {
    size_t idx, nidx;
    getIndices(x, idx, nidx);
    _points[idx] = y;
}

double BuiltTerrain::Height(double x) const {
    size_t idx, nidx;
    getIndices(x, idx, nidx);
    return Lerp(_points[idx], _points[nidx], PosFmod(x, _double_between_points));
}

double BuiltTerrain::RawHeight(double &x) const {
    size_t idx, nidx;
    getIndices(x, idx, nidx);
    x = x - PosFmod(x, _double_between_points);
    return _points[idx];
}

double BuiltTerrain::Slope(double x) const {
    size_t idx, nidx;
    getIndices(x, idx, nidx);
    return (_points[nidx] - _points[idx]) / _double_between_points;
}

V2d BuiltTerrain::Tangent(double x) const {
    V2d result;
    result.x = 1;
    result.y = Slope(x);
    return result.Normalized();
}

V2d BuiltTerrain::Normal(double x) const {
    V2d result = Tangent(x);
    double temp = result.x;
    result.x = -result.y;
    result.y = temp;
    return -result;
}
