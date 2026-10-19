#include "Camera.h"
#include <cmath>

Camera::Camera(const Vec3& pos, const Vec3& look_at, double fov_degrees, double aspect_ratio)
    : m_position(pos),
      m_target(look_at),
      m_fov_degrees(fov_degrees),
      m_aspect_ratio(aspect_ratio)
{
    update_basis();
}

void Camera::update_basis() {
    // 1. Calculate the forward view vector pointing from position toward target
    m_forward = (m_target - m_position).normalized();

    // 2. Build the right vector using the global "world up" direction (0, 1, 0)
    Vec3 world_up(0.0, 1.0, 0.0);

    // If the camera looks straight up or down along the Y axis, fall back to Z up
    // to avoid a zero-length cross product
    if (std::abs(m_forward.dot(world_up)) > 0.999) {
        world_up = Vec3(0.0, 0.0, 1.0);
    }

    m_right = m_forward.cross(world_up).normalized();

    // 3. Compute true local up vector perpendicular to forward and right
    m_up = m_right.cross(m_forward).normalized();

    // 4. Cache tan(fov / 2) to avoid re-evaluating trig for every pixel
    double fov_radians = m_fov_degrees * (M_PI / 180.0);
    m_viewport_scale = std::tan(fov_radians * 0.5);
}

Ray Camera::generate_ray(double pixel_x, double pixel_y, int screen_width, int screen_height) const {
    // Map screen pixel coords (0..width, 0..height) to normalized camera plane coordinates (-1..1)
    // Adding 0.5 samples the center of each pixel
    double u = (2.0 * (pixel_x + 0.5) / screen_width - 1.0) * m_viewport_scale * m_aspect_ratio;
    double v = (1.0 - 2.0 * (pixel_y + 0.5) / screen_height) * m_viewport_scale;

    // Direct ray along forward + local offset vectors, then normalize
    Vec3 ray_dir = (m_forward + (m_right * u) + (m_up * v)).normalized();

    return Ray(m_position, ray_dir);
}

void Camera::look_at(const Vec3& pos, const Vec3& target) {
    m_position = pos;
    m_target = target;
    update_basis();
}

void Camera::set_aspect_ratio(double aspect_ratio) {
    m_aspect_ratio = aspect_ratio;
}

void Camera::set_fov(double fov_degrees) {
    m_fov_degrees = fov_degrees;
    double fov_radians = m_fov_degrees * (M_PI / 180.0);
    m_viewport_scale = std::tan(fov_radians * 0.5);
}