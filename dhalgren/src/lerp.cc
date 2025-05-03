#include <cmath>

#include "lerp.h"

double Lerp(double from, double to, double factor) {
    return from + (to - from) * factor;
}

double LerpBetween(double a, double b, double current, double final) {
    return (a * ((final - current) / final) + b * (current / final));
}

double PiecewiseLerpBetween(const double *points, unsigned count, double current, double final) {
    double increment = final / (double)(count - 1);
    unsigned low_idx = 0;
    unsigned high_idx = 1;

    if (count < 2)
        return 0.0;

    for (; current > increment; current -= increment)
        low_idx = high_idx++;

    return LerpBetween(points[low_idx], points[high_idx], current, increment);
} 

double PosFmod(double d, double divisor) {
    d = std::fmod(d, divisor);
    if (d > 0.0)
        return d;
    return divisor + d;
}