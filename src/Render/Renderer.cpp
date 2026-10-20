#include "Renderer.h"
#include "Physics/Schwarzschild.h"
#include <algorithm>
#include <cmath>

namespace {
    // Primary image plus the lensed images formed by light that loops around the hole
    const int max_disk_crossings = 4;
}

Renderer::Renderer(const SceneConfig& config)
    : m_M(config.black_hole_mass),
      m_geodesics(config.black_hole_mass),
      m_disk(config.black_hole_mass,
             config.disk_inner_radius_in_M * config.black_hole_mass,
             config.disk_outer_radius_in_M * config.black_hole_mass,
             config.disk_peak_temperature),
      m_stars(0.6 * (config.fov_degrees * M_PI / 180.0) / config.screen_height)
{
    m_settings.exposure = config.exposure;
}

// The path crosses the disk plane (y = 0) wherever its position
// r (cos phi x_axis + sin phi y_axis) has no y component, which happens every half turn
Colour Renderer::shade_disk(const GeodesicTable::Path& path, double time, double& transmittance) const {
    Colour light;
    double a = path.x_axis.y;
    double c = path.y_axis.y;
    if (std::abs(a) + std::abs(c) < 1e-12) {
        return light; // The path lies in the disk plane itself, which is infinitely thin
    }

    double phi = std::fmod(std::atan2(-a, c), M_PI);
    if (phi < 0.0) {
        phi += M_PI;
    }
    if (phi < 1e-9) {
        phi += M_PI; // Camera itself is in the plane; skip its own position
    }

    // The photon's angular momentum about the disk axis, per unit energy. The traced path
    // runs backward in time, so the real photon's angular momentum points the other way.
    Vec3 normal = path.x_axis.cross(path.y_axis);
    double lambda_z = -path.impact_parameter * normal.y;

    for (int k = 0; k < max_disk_crossings; ++k, phi += M_PI) {
        double r = m_geodesics.radius_at(path, phi);
        if (r < 0.0) {
            break;
        }
        if (!m_disk.contains(r)) {
            continue;
        }

        Vec3 hit = path.x_axis * (r * std::cos(phi)) + path.y_axis * (r * std::sin(phi));
        double angle = std::atan2(hit.z, hit.x);
        double g = Schwarzschild::disk_frequency_shift(r, lambda_z, m_geodesics.camera_radius(), m_M);

        AccretionDisk::Sample s = m_disk.sample(r, angle, time, g);
        light += s.emission * transmittance;
        transmittance *= 1.0 - s.opacity;
        if (transmittance < 0.01) {
            break;
        }
    }
    return light;
}

Colour Renderer::shade(const GeodesicTable::Path& path, double time) const {
    double transmittance = 1.0;
    Colour light;

    if (m_settings.show_disk) {
        light = shade_disk(path, time, transmittance);
    }

    if (path.escaped && transmittance > 0.0) {
        Vec3 dir = m_geodesics.escape_direction(path);
        Colour sky = m_settings.show_grid ? m_grid.sample(dir) : m_stars.sample(dir);
        light += sky * transmittance;
    }
    return light;
}

void Renderer::render_hdr(const Camera& camera, double time, int width, int height) {
    m_hdr.resize(width * height);

    // Only retraces when the camera's distance from the hole changes
    m_geodesics.update(camera.get_position().norm());

    // Supersampling spreads n x n rays evenly across each pixel and averages them
    const int n = std::max(1, m_settings.supersampling);
    const double weight = 1.0 / (n * n);

    // Multi-threaded render pass using OpenMP across scanlines
    #pragma omp parallel for schedule(dynamic, 8)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Colour sum;
            for (int sy = 0; sy < n; ++sy) {
                for (int sx = 0; sx < n; ++sx) {
                    double px = x + (sx + 0.5) / n - 0.5;
                    double py = y + (sy + 0.5) / n - 0.5;
                    sum += shade(m_geodesics.trace(camera.generate_ray(px, py, width, height)), time);
                }
            }
            m_hdr[y * width + x] = sum * weight;
        }
    }

    if (m_settings.bloom) {
        m_bloom.apply(m_hdr, width, height, 0.8, 0.35);
    }
}
