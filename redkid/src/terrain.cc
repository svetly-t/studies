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

void BuiltTerrain::Initialize(size_t number_of_points, double double_between_points) {
    _number_of_points = number_of_points;
    _double_between_points = double_between_points;
    _points.assign(number_of_points, 0.0);
}

size_t BuiltTerrain::getIndex(double x) const {
    double index = _number_of_points * _double_between_points + x;
    // Lock index to range of vector
    if (index < 0.0)
        return 0;
    if (index > _number_of_points)
        return _number_of_points - 1;
    return (size_t)(index);
}

void BuiltTerrain::SetHeight(double x, double y) {
    size_t idx = getIndex(x);
    _points[idx] = y;
}

double BuiltTerrain::Height(double x) const {
    size_t idx = getIndex(x);
    return _points[idx];
}

double BuiltTerrain::Slope(double x) const {
    double result;
    size_t idx_at, idx_before, idx_after;

    idx_at = getIndex(x);
    idx_before = getIndex(x - _double_between_points);
    idx_after = getIndex(x + _double_between_points);

    result = (_points[idx_after] + _points[idx_at]) / _double_between_points;
    result += (_points[idx_at] + _points[idx_before]) / _double_between_points;
    result /= 2.0;

    return result;
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
    return result;
}

void TerrainBuilder::Initialize(size_t number_of_points, double double_between_points) {
    _terrain.Initialize(number_of_points, double_between_points);
}

void TerrainBuilder::Update(TerrainBuilder::UpdateContext *ctx) {
    V2d cursor_pos;
    V2d world_pos;
    cursor_pos.x = ctx->ks->mx;
    cursor_pos.y = ctx->ks->my;

    if (ctx->ks->mlc) {
        world_pos = ctx->camerap->ToWorldSpace(cursor_pos);
        _terrain.SetHeight(world_pos.x, world_pos.y);
    }
}

BuiltTerrain *TerrainBuilder::GetTerrain() {
    return &_terrain;
}
