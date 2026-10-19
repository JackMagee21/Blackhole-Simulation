#pragma once

#include "Math/Vec3.h"

struct Ray {
    Vec3 origin;
    Vec3 direction; // Always normalized (unit length = 1.0)

    constexpr Ray(const Vec3& origin, const Vec3& direction)
        : origin(origin), direction(direction) {}
};