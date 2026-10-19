#pragma once

#include <cstdint>
#include <algorithm>
#include <cmath>

struct Colour {
    double r;
    double g;
    double b;

    constexpr Colour(double r = 0.0, double g = 0.0, double b = 0.0)
        : r(r), g(g), b(b) {}

    // Colour addition (light accumulation)
    Colour operator+(const Colour& other) const {
        return {r + other.r, g + other.g, b + other.b};
    }

    // Scalar scaling (intensity, attenuation, exposure)
    Colour operator*(double scalar) const {
        return {r * scalar, g * scalar, b * scalar};
    }

    // Component-wise multiplication (filtering / material reflectance)
    Colour operator*(const Colour& other) const {
        return {r * other.r, g * other.g, b * other.b};
    }

    // Converts linear floating-point light values into a packed 32-bit integer (ARGB8888)
    // formatted for the SDL2 texture streaming pipeline.
    uint32_t to_argb32() const {
        // 1. Clamp values to [0.0, 1.0].
        // Relativistic beaming and Doppler boosts can exceed 1.0 (HDR values).
        // Clamping prevents integer overflow wrap-around (e.g. 260 becoming 4, 
        // which would cause blindingly bright regions to render as dark patches).
        double cr = std::clamp(r, 0.0, 1.0);
        double cg = std::clamp(g, 0.0, 1.0);
        double cb = std::clamp(b, 0.0, 1.0);

        // 2. Gamma correction: (1.0 / 2.2).
        // Physics equations work with linear light, but computer displays output light
        // using a non-linear power curve (~Display = Input^2.2). Raising our linear 
        // values to the reciprocal (1.0 / 2.2 ≈ 0.454) cancels this darkening out, 
        // preventing shadows and faint accretion disk glows from being crushed to black.
        double gr = std::pow(cr, 1.0 / 2.2);
        double gg = std::pow(cg, 1.0 / 2.2);
        double gb = std::pow(cb, 1.0 / 2.2);

        // 3. Scale normalized [0.0, 1.0] floats to standard 8-bit color channels [0, 255].
        uint8_t ir = static_cast<uint8_t>(gr * 255.0);
        uint8_t ig = static_cast<uint8_t>(gg * 255.0);
        uint8_t ib = static_cast<uint8_t>(gb * 255.0);

        // 4. Pack into a single 32-bit ARGB integer (0xAARRGGBB):
        // - 0xFFu << 24 sets the Alpha channel to 255 (fully opaque) in bits 24-31.
        // - ir << 16 places Red into bits 16-23.
        // - ig << 8 places Green into bits 8-15.
        // - ib sits directly in bits 0-7 (Blue).
        return (0xFFu << 24) | (ir << 16) | (ig << 8) | ib;
    }

    // Common presets
    static constexpr Colour black() { return {0.0, 0.0, 0.0}; }
    static constexpr Colour white() { return {1.0, 1.0, 1.0}; }

    static constexpr Colour Red() { return {1.0, 0.0, 0.0}; }
    static constexpr Colour Blue() { return {0.0, 1.0, 0.0}; }
    static constexpr Colour Green() { return {0.0, 0.0, 1.0}; }
};