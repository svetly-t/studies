#pragma once

#include "v2d.h"

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

class TerrainGenerator {
 public:
    void Init();
    struct UpdateContext {
        
    };
    void Update(UpdateContext *ctx);

};