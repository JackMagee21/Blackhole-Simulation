#pragma once

#include "Math/Vec3.h"
#include "Render/Colour.h"

// Procedural night sky at infinity: stars of random brightness and colour scattered
// over the celestial sphere, plus a faint galactic band.
class Starfield {
    public:
        // star_size is each star's angular radius in radians, about one pixel
        explicit Starfield(double star_size);

        Colour sample(const Vec3& dir) const;

    private:
        double m_star_size;
        Vec3 m_band_normal; // The galactic band lies around the great circle with this normal

        Colour stars(const Vec3& dir, int cells, double probability, double brightness, int layer) const;
        Colour galaxy(const Vec3& dir) const;
};
