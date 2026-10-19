#pragma once

#include "Colour.h"
#include "Physics/Vec3/Vec3.h"

// The infinitely distant background sky, sampled by the direction a ray escapes in.
// A checkerboard of latitude/longitude tiles makes lensing distortion easy to see.
class CelestialSphere {
    public:
        explicit CelestialSphere(double tile_scale = 6.0,
                                 const Colour& light = Colour(0.12, 0.14, 0.22),
                                 const Colour& dark = Colour(0.02, 0.02, 0.05));

        Colour sample(const Vec3& dir) const;

    private:
        double m_tile_scale; // Tiles per half-turn of longitude and per quarter-turn of latitude
        Colour m_light;
        Colour m_dark;
};
