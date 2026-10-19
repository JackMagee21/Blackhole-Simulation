#include "DeflectionTable.h"
#include <algorithm>
#include <cmath>

DeflectionTable::DeflectionTable(double M, int samples, const Physics::TracerSettings& tracer)
    : m_M(M),
      m_samples(samples),
      m_tracer(tracer),
      m_radius(-1.0),
      m_alpha_crit(0.0),
      m_exit_angle(samples, 0.0)
{
}

// Samples are packed quadratically toward alpha_crit, where rays wind around the photon
// sphere and the exit angle changes fastest
double DeflectionTable::sample_alpha(int i) const {
    double t = static_cast<double>(i + 1) / m_samples;
    return m_alpha_crit + (M_PI - m_alpha_crit) * t * t;
}

// Traces a ray in the plane's local 2D frame: hole at the origin, camera at (-r, 0),
// so alpha = 0 points straight at the hole and alpha = PI points directly away
bool DeflectionTable::trace_in_plane(double alpha, double& exit_angle) const {
    Vec3 pos(-m_radius, 0.0, 0.0);
    Vec3 dir(std::cos(alpha), std::sin(alpha), 0.0);

    Physics::PhotonResult result = Physics::trace_photon(pos, dir, m_M, m_tracer);
    if (result.captured) {
        return false;
    }

    exit_angle = std::atan2(result.exit_dir.y, result.exit_dir.x);
    return true;
}

void DeflectionTable::update(double camera_radius) {
    if (std::abs(camera_radius - m_radius) < 1e-9) {
        return;
    }
    m_radius = camera_radius;

    // 1. Captured rays are exactly those aimed within alpha_crit of the hole, so locate
    //    the shadow edge precisely by bisection using the same integrator
    double lo = 0.0;
    double hi = M_PI;
    double unused;
    for (int i = 0; i < 60; ++i) {
        double mid = 0.5 * (lo + hi);
        if (trace_in_plane(mid, unused)) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    m_alpha_crit = hi;

    // 2. Trace every escaping sample ray
    std::vector<char> escaped(m_samples);
    #pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < m_samples; ++i) {
        escaped[i] = trace_in_plane(sample_alpha(i), m_exit_angle[i]);
    }

    // 3. Patch any numerically trapped samples from their neighbour, then unwrap the
    //    angles so rays that loop around the hole interpolate smoothly. The last sample
    //    (alpha = PI, straight away from the hole) always escapes.
    for (int i = m_samples - 2; i >= 0; --i) {
        if (!escaped[i]) {
            m_exit_angle[i] = m_exit_angle[i + 1];
        }
        while (m_exit_angle[i] - m_exit_angle[i + 1] > M_PI) m_exit_angle[i] -= 2.0 * M_PI;
        while (m_exit_angle[i] - m_exit_angle[i + 1] < -M_PI) m_exit_angle[i] += 2.0 * M_PI;
    }
}

bool DeflectionTable::exit_direction(const Ray& ray, Vec3& out_dir) const {
    // Local frame of the ray's plane: e1 points toward the hole, e2 holds the rest of dir
    const Vec3& dir = ray.direction;
    Vec3 e1 = -ray.origin.normalized();
    double cos_alpha = std::clamp(dir.dot(e1), -1.0, 1.0);
    double alpha = std::acos(cos_alpha);

    if (alpha <= m_alpha_crit) {
        return false;
    }

    Vec3 perp = dir - e1 * cos_alpha;
    double perp_len2 = perp.norm_squared();

    // Pointing straight away from the hole: no bending
    if (perp_len2 < 1e-12) {
        out_dir = dir;
        return true;
    }
    Vec3 e2 = perp / std::sqrt(perp_len2);

    // Invert sample_alpha() to get a fractional table index, then interpolate
    double t = std::sqrt((alpha - m_alpha_crit) / (M_PI - m_alpha_crit)) * m_samples - 1.0;
    t = std::clamp(t, 0.0, static_cast<double>(m_samples - 1));
    int i = std::min(static_cast<int>(t), m_samples - 2);
    double f = t - i;
    double angle = m_exit_angle[i] * (1.0 - f) + m_exit_angle[i + 1] * f;

    out_dir = e1 * std::cos(angle) + e2 * std::sin(angle);
    return true;
}
