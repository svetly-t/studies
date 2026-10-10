#include "dude.h"
#include "collisions.h"
#include "level.h"

struct DudeCollision {
    LineToLineIntersection ground_isct;
};

void DudeCollisionCheck(
    Dude &dude,
    DudeCollision& dude_collision,
    DudeUpdateContext ctx
) {
    V2d downward;

    Level &level = *(ctx.level);
    LineToLineIntersection &ground_isct = dude_collision.ground_isct;
    V2d &pos = dude.pos;

    double dt = ctx.dt;
    double meters_per_pixel = ctx.meters_per_pixel;

    ground_isct.exists = false;

    for (auto &aabb: level.aabbs) {
        ground_isct = AABBToLineIntersect(aabb, pos, pos + downward);
        if (ground_isct.exists)
            break;
    }

    if (!ground_isct.exists) {
        // look for ropes here...
    }
}

void DudeUpdate(Dude &dude, DudeUpdateContext ctx) {
    DudeCollision dude_collision;

    switch (dude.state) {
        case Dude::RUNNING:
            DudeCollisionCheck(dude, dude_collision, ctx);
            break;
        case Dude::STANDING:
            break;
        default:
            break;
    }
}