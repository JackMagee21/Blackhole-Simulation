#include "../Window/Window.h"
#include "PixelBuffer/PixelBuffer.h"
#include "Colour.h"
#include "Physics/Vec3/Vec3.h"
#include "Physics/Ray.h"
#include "Camera/Camera.h"
#include <cmath>
#include <algorithm>

// Diagnostic hit test: Euclidean sphere of given radius centered at (0, 0, 0)
bool hit_sphere(const Ray& ray, double radius, double& t_hit) {
    double a = ray.direction.dot(ray.direction);
    double b = 2.0 * ray.origin.dot(ray.direction);
    double c = ray.origin.dot(ray.origin) - (radius * radius);

    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return false;

    double t = (-b - std::sqrt(discriminant)) / (2.0 * a);
    if (t > 0.0) {
        t_hit = t;
        return true;
    }
    return false;
}

int main() {
    const int screen_width = 800;
    const int screen_height = 600;
    const double aspect_ratio = static_cast<double>(screen_width) / screen_height;

    Window window("Black Hole Engine - Camera Verification", screen_width, screen_height);
    PixelBuffer buffer(window.get_renderer(), screen_width, screen_height);

    // Initial camera parameters
    Vec3 target(0.0, 0.0, 0.0);
    double fov_degrees = 60.0;
    double orbit_radius = 5.0;
    double orbit_angle = 0.0;

    Camera camera(Vec3(0.0, 1.0, -orbit_radius), target, fov_degrees, aspect_ratio);

    while (window.is_open()) {
        window.poll_events();

        // Slowly orbit camera around origin to confirm look_at and basis vectors work
        orbit_angle += 0.01;
        double cam_x = std::sin(orbit_angle) * orbit_radius;
        double cam_z = -std::cos(orbit_angle) * orbit_radius;
        camera.look_at(Vec3(cam_x, 1.2, cam_z), target);

        // Render pass: generate rays via the Camera class
        for (int y = 0; y < screen_height; ++y) {
            for (int x = 0; x < screen_width; ++x) {
                // All camera ray math is now encapsulated cleanly:
                Ray ray = camera.generate_ray(x, y, screen_width, screen_height);

                Colour pixel_color;
                double t_hit = 0.0;

                // Sphere stands in for the black hole coordinate center for now
                if (hit_sphere(ray, 1.2, t_hit)) {
                    Vec3 hit_point = ray.at(t_hit);
                    Vec3 normal = hit_point.normalized();

                    // Simple directional lighting
                    Vec3 light_dir = Vec3(1.0, 2.0, -1.0).normalized();
                    double diffuse = std::max(0.0, normal.dot(light_dir));

                    // Surface coloration with diffuse + ambient component
                    pixel_color = Colour(1.0, 0.5, 0.15) * (diffuse + 0.1);
                } else {
                    // Sky gradient relative to exit ray direction
                    double bg = (ray.direction.y + 1.0) * 0.5;
                    pixel_color = Colour(0.02, 0.02, 0.04) * (1.0 - bg) + Colour(0.06, 0.09, 0.18) * bg;
                }

                buffer.set_pixel(x, y, pixel_color);
            }
        }

        window.clear();
        buffer.render();
        window.present();
    }

    return 0;
}