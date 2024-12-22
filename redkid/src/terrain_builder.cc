#include "v2d.h"
#include "terrain.h"
#include "terrain_builder.h"

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
