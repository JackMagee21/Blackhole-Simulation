#pragma once

#include "Render/Bloom.h"
#include "Render/Colour.h"
#include "Scene/AccretionDisk.h"
#include "Scene/Camera.h"
#include "Scene/CelestialSphere.h"
#include "Scene/SceneConfig.h"
#include "Scene/Starfield.h"
#include "Physics/GeodesicTable.h"
#include <vector>

// Options that can change while running
struct RenderSettings {
    bool show_disk = true;
    bool show_grid = false; // Checkerboard sky instead of stars, to see the lensing distortion clearly
    bool bloom = true;
    double exposure = 1.0;
    int supersampling = 1;  // Rays per pixel along each axis; 2 or 3 smooths the photon ring and stars
};

// Renders the view of a Schwarzschild black hole with an accretion disk from a static camera
class Renderer {
    public:
        explicit Renderer(const SceneConfig& config);

        RenderSettings& settings() { return m_settings; }

        // Linear HDR light seen along one camera ray at a simulation time (in units of length, c = 1)
        Colour shade(const GeodesicTable::Path& path, double time) const;

        // Fills every pixel of target. Target is anything with get_width(), get_height() and
        // set_pixel(x, y, Colour), so a PixelBuffer works on screen and an Image works
        // for screenshots, with no SDL involved.
        template <typename Target>
        void render_frame(const Camera& camera, double time, Target& target) {
            const int width = target.get_width();
            const int height = target.get_height();
            render_hdr(camera, time, width, height);

            // Compress the HDR image into the range a screen can show
            #pragma omp parallel for
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    target.set_pixel(x, y, m_hdr[y * width + x].tone_mapped(m_settings.exposure));
                }
            }
        }

    private:
        double m_M;
        RenderSettings m_settings;
        GeodesicTable m_geodesics;
        AccretionDisk m_disk;
        Starfield m_stars;
        CelestialSphere m_grid;
        Bloom m_bloom;
        std::vector<Colour> m_hdr;

        void render_hdr(const Camera& camera, double time, int width, int height);
        Colour shade_disk(const GeodesicTable::Path& path, double time, double& transmittance) const;
};
