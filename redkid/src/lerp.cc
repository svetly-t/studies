#include <cmath>

#include "lerp.h"

double Lerp(double from, double to, double factor) {
    return from + (to - from) * factor;
}

double LerpBetween(double a, double b, double current, double final) {
    return (a * ((final - current) / final) + b * (current / final));
}

double PosFmod(double d, double divisor) {
    d = std::fmod(d, divisor);
    if (d > 0.0)
        return d;
    return divisor + d;
}