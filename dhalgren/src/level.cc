#include "level.h"
#include "sdl_state.h"

#include <cmath>
#include <cstdio>

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

void LevelInitialize(Level &level, int window_x, int window_y) {
    level.state = Level::READY_BOX;
}

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

void LevelSave(Level &level) {
    int written;
    char buf[256];
    const char *filename = "data.lvl";
    SDL_RWops *file = SDL_RWFromFile(filename, "r+");

    if (!file) {
        printf("%s does not exist, attempting to create...\n", filename);

        file = SDL_RWFromFile(filename, "w+");

        if (!file) {
            printf("cannot open %s, aborting...\n", filename);
            exit(1);
        }
    }

    for (AABB &aabb : level.aabbs) {
        written = snprintf(buf, sizeof(buf), "%lf,%lf,%lf,%lf\n", aabb.pos.x, aabb.pos.y, aabb.width, aabb.height);
        SDL_RWwrite(file, buf, written, 1);
    }

    SDL_RWclose(file);
}

void LevelLoad(Level &level) {
    AABB aabb;
    char buf[256];
    size_t current_offset = 0;
    size_t seek = 0;
    const char *filename = "data.lvl";
    SDL_RWops *file = SDL_RWFromFile(filename, "r");

    if (!file) {
        printf("%s does not exist, returning...\n", filename);
        return;
    }

    level.aabbs.clear();

    while (1) {
        current_offset = SDL_RWtell(file);
        seek = 0;

        if (SDL_RWread(file, buf, 1, sizeof(buf)) == 0) {
            printf("Error reading file: %s\n", SDL_GetError());
            break;
        }

        while (buf[seek++] != '\n') {}

        SDL_RWseek(file, current_offset + seek, RW_SEEK_SET);
        
        sscanf(buf, "%lf,%lf,%lf,%lf", &aabb.pos.x, &aabb.pos.y, &aabb.width, &aabb.height);
        
        level.aabbs.push_back(AABB{
            .pos = { aabb.pos.x, aabb.pos.y },
            .width = aabb.width,
            .height = aabb.height
        });
    }
}

bool LevelRemoveBox(Level &level, V2d mouse_pos) {
    for (size_t i = 0; i < level.aabbs.size(); ++i) {
        if (AABBToPointOverlap(level.aabbs[i], mouse_pos)) {
            level.aabbs[i] = level.aabbs[level.aabbs.size() - 1];
            level.aabbs.pop_back();
            return true;
        }
    }
    return false;
}

void LevelUpdate(Level &level, KeyState &ks, V2d &mouse_pos, double dt) {
    bool removed_box = false;
    switch (level.state) {
        case Level::READY_BOX:
            if (ks.ep != 0) {
                LevelSwitchState(level, Level::READY_LINE);
                break;
            }
            if (ks.mrcp != 0) {
                if (LevelRemoveBox(level, mouse_pos))
                    break;
            }
            if (ks.mlc != 0) {
                LevelSwitchState(level, Level::ADJUST_BOX);
                level.aabb.pos = mouse_pos;
                break;
            }
            if (ks.sp != 0) {
                LevelSave(level);
            }
            if (ks.lp != 0) {
                LevelLoad(level);
            }
            break;
        case Level::READY_LINE:
            if (ks.ep != 0) {
                LevelSwitchState(level, Level::READY_BOX);
                break;
            }
            if (ks.mlc != 0) {
                LevelSwitchState(level, Level::ADJUST_LINE);
                level.l1 = mouse_pos;
                break;
            }
            if (ks.sp != 0) {
                LevelSave(level);
            }
            if (ks.lp != 0) {
                LevelLoad(level);
            }
            break;
        case Level::ADJUST_LINE:
            level.l2 = mouse_pos;
            if (ks.mlc == 0) {
                LevelSwitchState(level, Level::READY_LINE);
                break;
            }
            break;
        case Level::ADJUST_BOX:
            level.aabb.width = mouse_pos.x - level.aabb.pos.x;
            level.aabb.height = mouse_pos.y - level.aabb.pos.y;
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