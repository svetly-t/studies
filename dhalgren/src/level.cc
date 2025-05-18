#include "level.h"

#include <cmath>

bool AABBToPointOverlap(AABB &aabb, V2d final) {
    if (final.y > aabb.pos.y &&
        final.x > aabb.pos.x &&
        final.y < aabb.pos.y + aabb.height &&
        final.x < aabb.pos.x + aabb.width)
            return true;
    return false;
}

bool LineToLineIntersect(V2d l1, V2d l2, V2d i1, V2d i2) {
    // if ((i1.y < l1.y && i2.y < l2.y && i1.y < l2.y && i2.y < l1.y) ||
    //     (i1.x < l1.x && i2.x < l2.x && i1.x < l2.x && i2.x < l1.x) ||
    //     (i1.y > l1.y && i2.y > l2.y && i1.y > l2.y && i2.y > l1.y) ||
    //     (i1.x > l1.x && i2.x > l2.x && i1.x > l2.x && i2.x > l1.x))
    //         return false;
    // if (std::copysign(1.0, (l2 - l1).Cross(i1)) != std::copysign(1.0, (l2 - l1).Cross(i2)))
    //     return true;
    // return false;
    double a1;
    double a2;
    double a3;
    double a4;
    a1 = (l2 - l1).Cross(i1 - l1);
    a2 = (l2 - l1).Cross(i2 - l1);
    if (std::copysign(1.0, a1) == std::copysign(1.0, a2))
        return false;
    a3 = std::abs((i2 - i1).Cross(l1 - i1));
    a4 = std::abs((i2 - i1).Cross(l2 - i1));
    if (a4 > a3)
        a3 = a4;
    return (a3 < std::abs(a1) + std::abs(a2));    
}

bool AABBToLineIntersect(AABB &aabb, Line& line, V2d i1, V2d i2) {
    V2d aabb_width = { aabb.width, 0.0 };
    V2d aabb_height = { 0.0, aabb.height };
    V2d aabb_corner = aabb.pos + aabb_width + aabb_height;
    if (LineToLineIntersect(aabb.pos, aabb.pos + aabb_width, i1, i2)) {
        line = { aabb.pos, aabb.pos + aabb_width };
        return true;
    }
    if (LineToLineIntersect(aabb.pos, aabb.pos + aabb_height, i1, i2)) {
        line = { aabb.pos, aabb.pos + aabb_height };
        return true;
    }
    if (LineToLineIntersect(aabb_corner, aabb_corner - aabb_width, i1, i2)) {
        line = { aabb_corner, aabb_corner - aabb_width };
        return true;
    }
    if (LineToLineIntersect(aabb_corner, aabb_corner - aabb_height, i1, i2)) {
        line = { aabb_corner, aabb_corner - aabb_height };
        return true;
    }
    return false;
}

void LevelSwitchState(Level &level, Level::State new_state) {
    level.state = new_state;
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
                LevelSwitchState(level, Level::READY_BOX);
                break;
            }
            break;
    }
}