#pragma once

#include <string>
#include <vector>

#include "v2d.h"
#include "sdl_state.h"
#include "camera.h"

class Terrain {
 public:
    virtual double Height(double pos) const;
    virtual double Slope(double pos) const;
    virtual V2d Tangent(double pos) const;
    virtual V2d Normal(double pos) const;
};

class DefaultTerrain : public Terrain {
 public:
    double Height(double pos) const override;
    double Slope(double pos) const override;
    V2d Tangent(double pos) const override;
    V2d Normal(double pos) const override;
};

class BuiltTerrain : public Terrain {
 public:
    double Height(double pos) const override;
    double Slope(double pos) const override;
    V2d Tangent(double pos) const override;
    V2d Normal(double pos) const override;
    void Initialize(size_t number_of_points, double double_between_points);
    inline void SetHeight(double x, double y);
 private:
    inline size_t getIndex(double x) const;
    size_t _number_of_points;
    double _double_between_points;
    std::vector<double> _points;
};

class TerrainBuilder {
 public:
    struct UpdateContext {
        KeyState *ks;
        Camera *camerap;
    };
    void Update(UpdateContext *ctx);
    void Initialize(size_t number_of_points, double double_between_points);
    BuiltTerrain *GetTerrain();
 private:
    BuiltTerrain _terrain;
};

class Cartographer {
 public:
    void Init();
    void LoadMap(std::string filename);
 private:
};