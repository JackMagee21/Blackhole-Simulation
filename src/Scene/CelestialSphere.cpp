#include "CelestialSphere.h"
#include <algorithm>
#include <cmath>

CelestialSphere::CelestialSphere(double tile_scale, const Colour& light, const Colour& dark)
    : m_tile_scale(tile_scale),
      m_light(light),
      m_dark(dark)
{
}

Colour CelestialSphere::sample(const Vec3& dir) const {
    // Convert direction vector to spherical coordinates (longitude / latitude)
    double phi = std::atan2(dir.z, dir.x);                    // -PI to PI
    double theta = std::asin(std::clamp(dir.y, -1.0, 1.0));   // -PI/2 to PI/2

    int u = static_cast<int>(std::floor((phi + M_PI) * (m_tile_scale / M_PI)));
    int v = static_cast<int>(std::floor((theta + M_PI * 0.5) * (m_tile_scale / (M_PI * 0.5))));

    bool is_light_tile = ((u + v) % 2 == 0);
    return is_light_tile ? m_light : m_dark;
}
