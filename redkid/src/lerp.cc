#include "lerp.h"

double Lerp(double from, double to, double factor) {
    return from + (to - from) * factor;
}