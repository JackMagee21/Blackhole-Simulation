#include "../Window/Window.h"
#include "PixelBuffer/PixelBuffer.h"
#include "Colour.h"
#include "Physics/Vec3/Vec3.h"
#include "Physics/Ray.h"
#include "Camera/Camera.h"
#include "Physics/DeflectionTable.h"
#include <cmath>
#include <algorithm>

// Samples a celestial sphere grid based on the final exit direction of the ray
Colour sample_celestial_background(const Vec3& dir) {
    // Convert direction vector to spherical coordinates (longitude / latitude)
    double phi = std::atan2(dir.z, dir.x);                    // -PI to PI
    double theta = std::asin(std::clamp(dir.y, -1.0, 1.0));   // -PI/2 to PI/2

    // Checkerboard/celestial coordinate lines to clearly reveal lensing distortion
    const double tile_scale = 6.0;
    int u = static_cast<int>(std::floor((phi + M_PI) * (tile_scale / M_PI)));
    int v = static_cast<int>(std::floor((theta + M_PI * 0.5) * (tile_scale / (M_PI * 0.5))));

    bool is_light_tile = ((u + v) % 2 == 0);

    // Subtle starfield-like colors
    if (is_light_tile) {
        return Colour(0.12, 0.14, 0.22);
    } else {
        return Colour(0.02, 0.02, 0.05);
    }
}

int main() {
    const int screen_width = 800;
    const int screen_height = 600;
    const double aspect_ratio = static_cast<double>(screen_width) / screen_height;

    Window window("Relativistic Black Hole - Schwarzschild Geodesics", screen_width, screen_height);
    PixelBuffer buffer(window.get_renderer(), screen_width, screen_height);

    Vec3 target(0.0, 0.0, 0.0);
    double fov_degrees = 55.0;
    double orbit_radius = 16.0;
    double orbit_angle = 0.0;

    Camera camera(Vec3(0.0, 2.0, -orbit_radius), target, fov_degrees, aspect_ratio);
    DeflectionTable deflection_table;

    while (window.is_open()) {
        window.poll_events();

        // Orbit the camera smoothly around the black hole
        orbit_angle += 0.008;
        double cam_x = std::sin(orbit_angle) * orbit_radius;
        double cam_z = -std::cos(orbit_angle) * orbit_radius;
        camera.look_at(Vec3(cam_x, 2.5, cam_z), target);

        // Only retraces when the camera's distance from the hole changes
        Vec3 cam_pos = camera.get_position();
        deflection_table.update(cam_pos.norm());

        // Multi-threaded render pass using OpenMP across scanlines
        #pragma omp parallel for schedule(dynamic, 8)
        for (int y = 0; y < screen_height; ++y) {
            for (int x = 0; x < screen_width; ++x) {
                Ray ray = camera.generate_ray(x, y, screen_width, screen_height);
                Vec3 exit_dir;
                Colour pixel_color = deflection_table.exit_direction(cam_pos, ray.direction, exit_dir)
                    ? sample_celestial_background(exit_dir)
                    : Colour::black();
                buffer.set_pixel(x, y, pixel_color);
            }
        }

        window.clear();
        buffer.render();
        window.present();
    }

    return 0;
}