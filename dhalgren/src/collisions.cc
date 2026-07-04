#include "collisions.h"

bool AABBToPointOverlap(AABB &aabb, V2d final) {
    if (final.y > aabb.pos.y &&
        final.x > aabb.pos.x &&
        final.y < aabb.pos.y + aabb.height &&
        final.x < aabb.pos.x + aabb.width)
            return true;
    return false;
}

// http://alienryderflex.com/polygon/
bool PointInPolygon(V2d point, V2d *vertices, int vertice_count) {
    double &x = point.x, &y = point.y;
    int i, j = vertice_count - 1;
    bool oddNodes=false;

    for (i = 0; i < vertice_count; i++) {
        if ((vertices[i].y < y && vertices[j].y >=y || vertices[j].y < y && vertices[i].y >=y) &&
            (vertices[i].x <= x || vertices[j].x <= x)) {
            if (vertices[i].x + (y - vertices[i].y) / (vertices[j].y - vertices[i].y) * (vertices[j].x - vertices[i].x) < x) {
                oddNodes=!oddNodes;
            }
        }
        j=i;
    }

    return oddNodes;
}

// This isn't really AABB-to-AABB; if reticle is entirely within the target it fails
bool AABBToAABBOverlap(AABB target, AABB reticle, V2d &overlap) {
    if (AABBToPointOverlap(reticle, target.pos)) {
        overlap = target.pos;
        return true;
    }
    if (AABBToPointOverlap(reticle, target.pos + V2d(target.width, 0))) {
        overlap = target.pos + V2d(target.width, 0);
        return true;
    }
    if (AABBToPointOverlap(reticle, target.pos + V2d(target.width, target.height))) {
        overlap = target.pos + V2d(target.width, target.height);
        return true;
    }
    if (AABBToPointOverlap(reticle, target.pos + V2d(0, target.height))) {
        overlap = target.pos + V2d(0, target.height);
        return true;
    }
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
