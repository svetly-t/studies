#pragma once

#include <vector>

#include "v2d.h"

class Terrain {
 public:
    virtual double Height(double pos) const = 0;
    virtual double Slope(double pos) const = 0;
    virtual V2d Tangent(double pos) const = 0;
    virtual V2d Normal(double pos) const = 0;
    ~Terrain() {}
};

class DefaultTerrain : public Terrain {
 public:
    double Height(double x) const override;
    double Slope(double x) const override;
    V2d Tangent(double x) const override;
    V2d Normal(double x) const override;
};

class BuiltTerrain : public Terrain {
 public:
    double Height(double x) const override;
    double Slope(double x) const override;
    V2d Tangent(double x) const override;
    V2d Normal(double x) const override;
    void Initialize(size_t number_of_points, double double_between_points);
    void SetHeight(double x, double y);
 private:
    inline size_t getIndex(double x) const;
    size_t _number_of_points;
    double _double_between_points;
    std::vector<double> _points;
};
