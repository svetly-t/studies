#include "v2d.h"
#include "lerp.h"
#include "terrain.h"
#include "terrain_builder.h"

void TerrainBuilder::Initialize(size_t number_of_points, double double_between_points) {
    _terrain.Initialize(number_of_points, double_between_points);
}

void TerrainBuilder::Update(TerrainBuilder::UpdateContext *ctx) {
    bool done;
    double terrain_scale;
    double lerp_factor;
    
    V2d cursor_pos_lerp;
    V2d cursor_pos;
    cursor_pos = {(double)ctx->ks->mx, (double)ctx->ks->my};
    cursor_pos = ctx->camerap->ToWorldSpace(cursor_pos);
    
    if (ctx->ks->mlc) {
        done = false;
        terrain_scale = _terrain.GetScale();

        cursor_pos_lerp = _last_cursor_pos;

        if (cursor_pos_lerp.x == cursor_pos.x)
            _terrain.SetHeight(cursor_pos.x, cursor_pos.y);
        
        for (; cursor_pos_lerp.x < cursor_pos.x; cursor_pos_lerp.x += terrain_scale) {
            done = true;
            lerp_factor = (cursor_pos_lerp.x - _last_cursor_pos.x) / (cursor_pos.x - _last_cursor_pos.x);
            cursor_pos_lerp.y = Lerp(cursor_pos_lerp.y, cursor_pos.y, lerp_factor);
            _terrain.SetHeight(cursor_pos_lerp.x, cursor_pos_lerp.y);
        }

        for (; !done && cursor_pos_lerp.x > cursor_pos.x; cursor_pos_lerp.x -= terrain_scale) {
            lerp_factor = (cursor_pos_lerp.x - _last_cursor_pos.x) / (cursor_pos.x - _last_cursor_pos.x);
            cursor_pos_lerp.y = Lerp(cursor_pos_lerp.y, cursor_pos.y, lerp_factor);
            _terrain.SetHeight(cursor_pos_lerp.x, cursor_pos_lerp.y);
        }
    }

    _last_cursor_pos = cursor_pos;
}

BuiltTerrain *TerrainBuilder::GetTerrain() {
    return &_terrain;
}
