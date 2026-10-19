#pragma once

#include "Render/Colour.h"
#include "Scene/Camera.h"
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
        Colour shade(const Ray& ray) const {
            Vec3 exit_dir;
            return m_deflection_table.exit_direction(ray, exit_dir)
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
            m_deflection_table.update(camera.get_position().norm());

            // Multi-threaded render pass using OpenMP across scanlines
            #pragma omp parallel for schedule(dynamic, 8)
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    target.set_pixel(x, y, shade(camera.generate_ray(x, y, width, height)));
                }
            }
        }

    private:
        DeflectionTable m_deflection_table;
        CelestialSphere m_sky;
};
