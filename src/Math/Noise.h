#pragma once

#include <cmath>
#include <cstdint>

// Small deterministic hash and value noise for procedural textures (stars, disk turbulence)
namespace Noise {

    inline uint32_t hash(uint32_t x) {
        x ^= x >> 16;
        x *= 0x7feb352dU;
        x ^= x >> 15;
        x *= 0x846ca68bU;
        x ^= x >> 16;
        return x;
    }

    inline uint32_t hash(int x, int y, int z) {
        uint32_t h = hash(static_cast<uint32_t>(x));
        h = hash(h + static_cast<uint32_t>(y) * 0x9e3779b9U);
        return hash(h + static_cast<uint32_t>(z) * 0x85ebca6bU);
    }

    // Uniform random number in [0, 1) from a hash
    inline double to_unit(uint32_t h) {
        return (h >> 8) * (1.0 / 16777216.0);
    }

    // Smoothly interpolated random values on an integer lattice, in [0, 1]
    inline double value(double x, double y, double z) {
        double fx = std::floor(x);
        double fy = std::floor(y);
        double fz = std::floor(z);
        int ix = static_cast<int>(fx);
        int iy = static_cast<int>(fy);
        int iz = static_cast<int>(fz);

        // Smoothstep weights hide the lattice grid
        double tx = x - fx;
        double ty = y - fy;
        double tz = z - fz;
        tx = tx * tx * (3.0 - 2.0 * tx);
        ty = ty * ty * (3.0 - 2.0 * ty);
        tz = tz * tz * (3.0 - 2.0 * tz);

        auto corner = [&](int dx, int dy, int dz) {
            return to_unit(hash(ix + dx, iy + dy, iz + dz));
        };
        auto lerp = [](double a, double b, double t) { return a + (b - a) * t; };

        double x00 = lerp(corner(0, 0, 0), corner(1, 0, 0), tx);
        double x10 = lerp(corner(0, 1, 0), corner(1, 1, 0), tx);
        double x01 = lerp(corner(0, 0, 1), corner(1, 0, 1), tx);
        double x11 = lerp(corner(0, 1, 1), corner(1, 1, 1), tx);
        return lerp(lerp(x00, x10, ty), lerp(x01, x11, ty), tz);
    }

    // Fractal sum of octaves: coarse shapes with finer detail on top, in [0, 1]
    inline double fbm(double x, double y, double z, int octaves) {
        double sum = 0.0;
        double amplitude = 0.5;
        double total = 0.0;
        for (int i = 0; i < octaves; ++i) {
            sum += amplitude * value(x, y, z);
            total += amplitude;
            x *= 2.03;
            y *= 2.03;
            z *= 2.03;
            amplitude *= 0.5;
        }
        return sum / total;
    }

} // namespace Noise
