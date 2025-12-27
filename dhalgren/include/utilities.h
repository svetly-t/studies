#pragma once

#include "v2d.h"

double signOf(double d);

double degToRad(double degree);

double determinant2d(double a11, double a12, double a21, double a22);

double determinant3d(double A[][3]);

void findIntersection(V2d p1, V2d p2, V2d p3, double &a, double &b, double &c);

double singleRopeConstraint(V2d &pos1, V2d &pos2, double w1, double w2, double dist);
