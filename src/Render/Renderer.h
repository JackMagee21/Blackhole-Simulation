#pragma once

#include "Colour.h"
#include "Camera/Camera.h"
#include "Physics/DeflectionTable.h"
#include "Scene/CelestialSphere.h"

// Renders the lensed view of a Schwarzschild black hole from a camera.
class Renderer {
    public:
        explicit Renderer(double black_hole_mass, const CelestialSphere& sky = CelestialSphere())
            : m_deflection_table(black_hole_mass),
              m_sky(sky)
        {
        }

        // Colour seen along one camera ray
        Colour shade(const Vec3& cam_pos, const Vec3& dir) const {
            Vec3 exit_dir;
            return m_deflection_table.exit_direction(cam_pos, dir, exit_dir)
                ? m_sky.sample(exit_dir)
                : Colour::black();
        }

        // Fills every pixel of target. Target is anything with get_width(), get_height() and
        // set_pixel(x, y, Colour), so a PixelBuffer works on screen and a plain CPU image
        // works in headless tests or screenshots, with no SDL involved.
        template <typename Target>
        void render_frame(const Camera& camera, Target& target) {
            const int width = target.get_width();
            const int height = target.get_height();

            // Only retraces when the camera's distance from the hole changes
            Vec3 cam_pos = camera.get_position();
            m_deflection_table.update(cam_pos.norm());

            // Multi-threaded render pass using OpenMP across scanlines
            #pragma omp parallel for schedule(dynamic, 8)
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    Ray ray = camera.generate_ray(x, y, width, height);
                    target.set_pixel(x, y, shade(cam_pos, ray.direction));
                }
            }
        }

    private:
        DeflectionTable m_deflection_table;
        CelestialSphere m_sky;
};
