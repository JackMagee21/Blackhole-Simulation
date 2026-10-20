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

    Colour& operator+=(const Colour& other) {
        r += other.r;
        g += other.g;
        b += other.b;
        return *this;
    }

    // Scalar scaling (intensity, attenuation, exposure)
    Colour operator*(double scalar) const {
        return {r * scalar, g * scalar, b * scalar};
    }

    // Component-wise multiplication (filtering / material reflectance)
    Colour operator*(const Colour& other) const {
        return {r * other.r, g * other.g, b * other.b};
    }

    // Perceived brightness of linear sRGB light (Rec. 709 weights)
    double luminance() const {
        return 0.2126 * r + 0.7152 * g + 0.0722 * b;
    }

    // Filmic tone curve (Narkowicz's ACES fit). Physical light spans a huge range of
    // brightness; this rolls very bright values off smoothly toward white instead of
    // hard-clipping them, so the hot inner disk keeps its shape and colour.
    Colour tone_mapped(double exposure) const {
        auto aces = [](double x) {
            return (x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14);
        };
        return {aces(r * exposure), aces(g * exposure), aces(b * exposure)};
    }

    // Display transform: maps linear light to gamma-encoded values in [0, 1].
    // HDR values should go through tone_mapped() first.
    Colour to_display() const {
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

        return {gr, gg, gb};
    }

    // Packs channels already in [0, 1] into a 32-bit integer (ARGB8888)
    // formatted for the SDL2 texture streaming pipeline.
    uint32_t pack_argb32() const {
        // 1. Scale normalized [0.0, 1.0] floats to standard 8-bit color channels [0, 255].
        uint8_t ir = static_cast<uint8_t>(r * 255.0);
        uint8_t ig = static_cast<uint8_t>(g * 255.0);
        uint8_t ib = static_cast<uint8_t>(b * 255.0);

        // 2. Pack into a single 32-bit ARGB integer (0xAARRGGBB):
        // - 0xFFu << 24 sets the Alpha channel to 255 (fully opaque) in bits 24-31.
        // - ir << 16 places Red into bits 16-23.
        // - ig << 8 places Green into bits 8-15.
        // - ib sits directly in bits 0-7 (Blue).
        return (0xFFu << 24) | (ir << 16) | (ig << 8) | ib;
    }

    // Converts linear light straight to a packed display pixel
    uint32_t to_argb32() const {
        return to_display().pack_argb32();
    }

    static constexpr Colour black() { return {0.0, 0.0, 0.0}; }
};
