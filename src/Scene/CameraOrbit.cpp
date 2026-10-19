#include "CameraOrbit.h"
#include <cmath>

CameraOrbit::CameraOrbit(double radius, double height, double angular_speed, const Vec3& target)
    : m_radius(radius),
      m_height(height),
      m_angular_speed(angular_speed),
      m_angle(0.0),
      m_target(target)
{
}

void CameraOrbit::advance(double dt_seconds) {
    // Wrap to keep the angle small so sin/cos stay precise over long runs
    m_angle = std::fmod(m_angle + m_angular_speed * dt_seconds, 2.0 * M_PI);
}

Vec3 CameraOrbit::position() const {
    double x = std::sin(m_angle) * m_radius;
    double z = -std::cos(m_angle) * m_radius;
    return m_target + Vec3(x, m_height, z);
}
