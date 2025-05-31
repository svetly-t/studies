#include "level.h"
#include "sdl_state.h"

#include <cmath>

bool AABBToPointOverlap(AABB &aabb, V2d final) {
    if (final.y > aabb.pos.y &&
        final.x > aabb.pos.x &&
        final.y < aabb.pos.y + aabb.height &&
        final.x < aabb.pos.x + aabb.width)
            return true;
    return false;
}

V2d LineToLineProjectionPoint(V2d l1, V2d l2, V2d i1, V2d i2) {
    V2d tangent = (l2 - l1).Normalized();
    return l1 + tangent * ((i2 - l1) * tangent);
}

V2d LineToLineIntersectionPoint(V2d l1, V2d l2, V2d i1, V2d i2) {
    V2d normal = (l2 - l1).Orthogonal();
    double proj_i2 = std::abs((i2 - l1) * normal);
    double proj_i1 = std::abs((i1 - l1) * normal);
    double ratio = proj_i1 / (proj_i1 + proj_i2);

    return i1 + ((i2 - i1) * ratio);
}

V2d LineToLineIntersectionNormal(V2d l1, V2d l2, V2d i1, V2d i2) {
    V2d normal = (l2 - l1).Orthogonal();
    return normal.Normalized(); // * std::copysign(1.0, (l2-l1).Cross(i1-l1)) * -1.0 /* I don't know why this -1.0 is needed */;
}

LineToLineIntersection LineToLineIntersect(V2d l1, V2d l2, V2d i1, V2d i2) {
    double a1;
    double a2;
    double a3;
    double a4;

    LineToLineIntersection isct;

    isct.exists = false;
    isct.l1 = l1;
    isct.l2 = l2;

    a1 = (l2 - l1).Cross(i1 - l1);
    a2 = (l2 - l1).Cross(i2 - l1);
    if (std::copysign(1.0, a1) == std::copysign(1.0, a2))
        return isct;

    a3 = std::abs((i2 - i1).Cross(l1 - i1));
    a4 = std::abs((i2 - i1).Cross(l2 - i1));
    if (a4 > a3)
        a3 = a4;

    if (!(a3 < std::abs(a1) + std::abs(a2)))
        return isct;

    isct.exists = true;
    isct.intersection_point = LineToLineIntersectionPoint(l1, l2, i1, i2);
    isct.projection_point = LineToLineProjectionPoint(l1, l2, i1, i2);
    isct.normal = LineToLineIntersectionNormal(l1, l2, i1, i2);

    return isct;
}

LineToLineIntersection AABBToLineIntersect(AABB &aabb, V2d i1, V2d i2) {
    V2d aabb_width = { aabb.width, 0.0 };
    V2d aabb_height = { 0.0, aabb.height };

    LineToLineIntersection isct;

    isct = LineToLineIntersect(aabb.pos, aabb.pos + aabb_width, i1, i2);
    if (isct.exists)
        return isct;
    isct = LineToLineIntersect(aabb.pos + aabb_width, aabb.pos + aabb_width + aabb_height, i1, i2);
    if (isct.exists)
        return isct;
    isct = LineToLineIntersect(aabb.pos + aabb_width + aabb_height, aabb.pos + aabb_height, i1, i2);
    if (isct.exists)
        return isct;
    isct = LineToLineIntersect(aabb.pos + aabb_height, aabb.pos, i1, i2);
    if (isct.exists)
        return isct;

    return isct;
}

LineToLineIntersection AABBToLineIntersect(AABB &aabb, Line l) {
    return AABBToLineIntersect(aabb, l.p1, l.p2);
}

void LevelInitialize(Level &level, int window_x, int window_y) {}

void LevelSwitchState(Level &level, Level::State new_state) {
    level.state = new_state;
}

uint64_t LevelChunkMapIndex(double x, double y) {
    int ix = (int)x;
    int iy = (int)y;

    ix = ix - (ix % Chunk::kWidth);
    iy = iy - (iy % Chunk::kHeight);

    return ((uint64_t)iy << 31) | (uint64_t)ix;
}

void LevelUpdate(Level &level, KeyState &ks, double dt) {
    switch (level.state) {
        case Level::READY_BOX:
            if (ks.ep != 0) {
                LevelSwitchState(level, Level::READY_LINE);
                break;
            }
            if (ks.mlc != 0) {
                LevelSwitchState(level, Level::ADJUST_BOX);
                level.aabb.pos.x = ks.mx;
                level.aabb.pos.y = ks.my;
                break;
            }
            break;
        case Level::READY_LINE:
            if (ks.ep != 0) {
                LevelSwitchState(level, Level::READY_BOX);
                break;
            }
            if (ks.mlc != 0) {
                LevelSwitchState(level, Level::ADJUST_LINE);
                level.l1.x = ks.mx;
                level.l1.y = ks.my;
                break;
            }
            break;
        case Level::ADJUST_LINE:
            level.l2.x = ks.mx;
            level.l2.y = ks.my;
            if (ks.mlc == 0) {
                LevelSwitchState(level, Level::READY_LINE);
                break;
            }
            break;
        case Level::ADJUST_BOX:
            level.aabb.width = ks.mx - level.aabb.pos.x;
            level.aabb.height = ks.my - level.aabb.pos.y;
            if (ks.mlc == 0) {
                if (level.aabb.width < 0.0) {
                    level.aabb.pos.x += level.aabb.width;
                    level.aabb.width = -level.aabb.width;
                }
                if (level.aabb.height < 0.0) {
                    level.aabb.pos.y += level.aabb.height;
                    level.aabb.height = -level.aabb.height;
                }
                level.aabbs.push_back(level.aabb);
                LevelSwitchState(level, Level::READY_BOX);
                break;
            }
            break;
    }
}