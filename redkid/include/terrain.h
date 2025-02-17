#pragma once

#include <vector>

#include "v2d.h"

class Terrain {
 public:
    virtual double Height(double x) const = 0;
    virtual double RawHeight(double &x) const = 0;
    virtual double Slope(double x) const = 0;
    virtual V2d Tangent(double x) const = 0;
    virtual V2d Normal(double x) const = 0;
    virtual double LeftBound() const = 0;
    virtual double RightBound() const = 0;
    virtual V2d HighestPoint() const = 0;
    ~Terrain() {}
};

class DefaultTerrain : public Terrain {
 public:
    double Height(double x) const override;
    double RawHeight(double &x) const override;
    double Slope(double x) const override;
    V2d Tangent(double x) const override;
    V2d Normal(double x) const override;
};

class BuiltTerrain : public Terrain {
 public:
    double Height(double x) const override;
    double RawHeight(double &x) const override;
    double Slope(double x) const override;
    V2d Tangent(double x) const override;
    V2d Normal(double x) const override;
    double LeftBound() const override;
    double RightBound() const override;
    V2d HighestPoint() const override;
    void Initialize(size_t number_of_points, double double_between_points);
    void SetHeight(double x, double y);
    double GetScale();
 private:
    inline void getIndices(double x, size_t &floor, size_t &ceil) const;
    size_t _number_of_points;
    double _double_between_points;
    V2d _highest_point;
    std::vector<double> _points;
};
