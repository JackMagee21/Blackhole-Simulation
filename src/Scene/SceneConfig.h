#pragma once

// Every tunable setting for a run, in one place
struct SceneConfig {
    int screen_width = 800;
    int screen_height = 600;
    double fov_degrees = 55.0;

    // Black hole mass in geometric units (G = c = 1). Event horizon radius is 2M.
    double black_hole_mass = 0.25;

    // Camera orbit distances are multiples of M, so changing the mass scales the hole
    // without changing the view. Shrink orbit_radius_in_M to make the hole look bigger.
    double orbit_radius_in_M = 16.0;
    double camera_height_in_M = 2.5;
    double orbit_speed = 0.48; // Radians per second

    double aspect_ratio() const {
        return static_cast<double>(screen_width) / screen_height;
    }
};
