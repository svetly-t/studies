#pragma once

#include <cmath>

class V2d {
 public:
    V2d() {
        x = 0;
        y = 0;
    }
    V2d(double ix, double iy) {
        x = ix;
        y = iy;
    }
    double operator*(const V2d &other) {
        return x * other.x + y * other.y;
    }
    double operator^(const V2d &other) {
        return x * other.y + y * other.x;
    }
    V2d& operator+=(const V2d& o) {
        x += o.x;
        y += o.y;
        return *this;
    }
    V2d& operator-=(const V2d& o) {
        x -= o.x;
        y -= o.y;
        return *this;
    }
    V2d& operator/=(const double& d) {
        x /= d;
        y /= d;
        return *this;
    }
    V2d& operator*=(const double& d) {
        x *= d;
        y *= d;
        return *this;
    }
    V2d operator*(const double& d) const {
        V2d result;
        result.x = x * d;
        result.y = y * d;
        return result;
    }
    V2d operator/(const double& d) const {
        V2d result;
        result.x = x / d;
        result.y = y / d;
        return result;
    }
    V2d operator+(const V2d& o) const {
        V2d result;
        result.x = x + o.x;
        result.y = y + o.y;
        return result;
    }
    V2d operator-(const V2d& o) const {
        V2d result;
        result.x = x - o.x;
        result.y = y - o.y;
        return result;
    }
    double operator*(const V2d& o) const {
        return x * o.x + y * o.y;
    }
    // Unary negative operator
    V2d operator-() const {
        V2d v;
        v.x = -x;
        v.y = -y;
        return v;
    }
    double Magnitude() {
        return sqrt(x * x + y * y);
    }
    V2d Normalized() {
        if (!this->Magnitude())
            return {0.0, 0.0}; 
        return *this / this->Magnitude();
    }
    double x;
    double y;
};
