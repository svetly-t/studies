#pragma once

#include <string>

#include "v2d.h"
#include "sdl_state.h"
#include "camera.h"
#include "terrain.h"

class TerrainBuilder {
 public:
    struct UpdateContext {
        KeyState *ks;
        Camera *camerap;
    };
    void Update(UpdateContext *ctx);
    void Initialize(size_t number_of_points, double double_between_points);
    BuiltTerrain *GetTerrain();
 private:
    BuiltTerrain _terrain;
    V2d _last_cursor_pos;
};

class Cartographer {
 public:
    void Init();
    void LoadMap(std::string filename);
 private:
};