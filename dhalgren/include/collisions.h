#pragma once

#include "v2d.h"

struct Line {
    V2d p1;
    V2d p2;
};

struct AABB {
    V2d pos;
    double width;
    double height;
};

struct Circle {
    V2d pos;
    double radius;
    double sqrRadius;
};

bool PointInPolygon(V2d point, V2d *vertices, int vertice_count);

bool AABBToPointOverlap(AABB &aabb, V2d final);

bool AABBToAABBOverlap(AABB target, AABB sweep, V2d &overlap);

struct LineToLineIntersection {
    bool exists;
    V2d l1;
    V2d l2;
    V2d intersection_point;
    V2d projection_point;
    V2d normal;
};

LineToLineIntersection AABBToLineIntersect(AABB &aabb, V2d i1, V2d i2);

LineToLineIntersection AABBToLineIntersect(AABB &aabb, Line l);
