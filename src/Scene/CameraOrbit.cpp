#include "CameraOrbit.h"
#include <algorithm>
#include <cmath>

namespace {
    // Stay just short of straight up or down, where the camera's "up" direction flips
    const double max_elevation = 85.0 * M_PI / 180.0;
}

CameraOrbit::CameraOrbit(double radius, double elevation_degrees, double angular_speed,
                         double min_radius, double max_radius, const Vec3& target)
    : m_radius(radius),
      m_elevation(elevation_degrees * M_PI / 180.0),
      m_azimuth(0.0),
      m_angular_speed(angular_speed),
      m_min_radius(min_radius),
      m_max_radius(max_radius),
      m_auto_rotate(true),
      m_target(target)
{
}

void CameraOrbit::advance(double dt_seconds) {
    if (m_auto_rotate) {
        rotate(m_angular_speed * dt_seconds, 0.0);
    }
}

void CameraOrbit::rotate(double d_azimuth, double d_elevation) {
    // Wrap to keep the angle small so sin/cos stay precise over long runs
    m_azimuth = std::fmod(m_azimuth + d_azimuth, 2.0 * M_PI);
    m_elevation = std::clamp(m_elevation + d_elevation, -max_elevation, max_elevation);
}

void CameraOrbit::zoom(double factor) {
    m_radius = std::clamp(m_radius * factor, m_min_radius, m_max_radius);
}

Vec3 CameraOrbit::position() const {
    double horizontal = std::cos(m_elevation) * m_radius;
    double x = std::sin(m_azimuth) * horizontal;
    double y = std::sin(m_elevation) * m_radius;
    double z = -std::cos(m_azimuth) * horizontal;
    return m_target + Vec3(x, y, z);
}
