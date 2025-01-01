#pragma once

double Lerp(double from, double to, double factor);

double LerpBetween(double a, double b, double current, double final);

double PiecewiseLerpBetween(const double *points, unsigned count, double current, double final);

double PosFmod(double d, double divisor);