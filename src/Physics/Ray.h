#pragma once

#include "Vec3/Vec3.h"

struct Ray {
    Vec3 origin;
    Vec3 direction; // Always normalized (unit length = 1.0)

    constexpr Ray(const Vec3& origin, const Vec3& direction)
        : origin(origin), direction(direction) {}

    // Position along the ray at distance t: p(t) = origin + t * direction
    Vec3 at(double t) const {
        return origin + direction * t;
    }
};