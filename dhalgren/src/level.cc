#include "level.h"
#include "rope.h"
#include "sdl_state.h"
#include "utilities.h"

#include <cmath>
#include <cstdlib>
#include <cstdio>

void LevelInitialize(Level &level, int window_x, int window_y) {
    level.state = Level::RANDOM_POPULATE_START;
    level.window_x = window_x;
    level.window_y = window_y;
}

void LevelSwitchState(Level &level, Level::State new_state) {
    level.state = new_state;
}

void LevelRandomPopulate(Level &level, RopeState &rs) {
    double x, y, width, height;

    level.aabbs.clear();
    level.aabb.height = 0;
    level.aabb.width = 0;

    // Always place a box directly beneath origin so that the player has something to fall on
    x = -50;
    y = 100;
    width = 100.0;
    height = 100.0;
    level.aabbs.push_back(AABB{
        .pos = { x, y },
        .width = width,
        .height = height
    });

    for (int i = 1; i < 10; ++i) {
        AABB test;
        V2d test_overlap;
        bool overlap = false;

        x = rand() % level.window_x - level.window_x / 2;
        y = rand() % level.window_y - level.window_y / 2;
        width = rand() % 100 + 100;
        height = rand() % 100 + 100;

        test = AABB{
            .pos = { x, y },
            .width = width,
            .height = height,
        };

        // Make sure that none of the corners overlap with any of the other boxes
        // This doesn't totally prevent overlap
        for (auto &aabb : level.aabbs) {
            if (AABBToAABBOverlap(aabb, test, test_overlap) ||
                AABBToAABBOverlap(test, aabb, test_overlap)) {
                overlap = true;
                break;
            }
        }

        if (overlap) {
            --i;
            continue;
        }

        level.aabbs.push_back(AABB{
            .pos = { x, y },
            .width = width,
            .height = height,
        });

        if (rand() % 8 == 0 && i > 1) {
            // RopeAdd(rs, level.aabbs[i].pos, level.aabbs[i - 1].pos, 10, false, false, {0, 0});
            // Old pole creation logic:
            // RopeCreate(rs, level.aabbs[i].pos - V2d(40.0, 80.0), level.aabbs[i].pos, 10, false, true, {0, 0});
        }

        if (rand() % 4 == 0) {
            level.circles.push_back(Circle{
                .pos = { x - 20.0, y - 20.0 },
                .radius = 20.0,
            });
        }
    }
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

void LevelUpdate(Level &level, RopeState &rs, KeyState &ks, V2d mouse_pos, double dt) {
    bool removed_box = false;
    switch (level.state) {
        case Level::READY_BOX:
            // if (ks.ep != 0) {
            //     LevelSwitchState(level, Level::READY_LINE);
            //     break;
            // }
            if (ks.mrcp != 0) {
                if (LevelRemoveBox(level, mouse_pos))
                    break;
            }
            if (ks.mlc != 0) {
                // LevelSwitchState(level, Level::ADJUST_BOX);
                // level.aabb.pos = mouse_pos;
                break;
            }
            if (ks.rp != 0) {
                LevelSwitchState(level, Level::RANDOM_POPULATE_START);
                break;
            }
            if (ks.sp != 0) {
                LevelSave(level);
            }
            if (ks.lp != 0) {
                LevelLoad(level);
            }
            break;
        case Level::RANDOM_POPULATE_START:
            RopeStateInitialize(rs);
            LevelRandomPopulate(level, rs);
            LevelSwitchState(level, Level::RANDOM_POPULATE_DONE);
            break;
        case Level::RANDOM_POPULATE_DONE:
            if (ks.rp == 0) {
                LevelSwitchState(level, Level::READY_BOX);
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
