#pragma once

// Every tunable setting for a run, in one place
struct SceneConfig {
    int screen_width = 800;
    int screen_height = 600;
    double fov_degrees = 50.0;

    // Black hole mass in geometric units (G = c = 1). Event horizon radius is 2M.
    double black_hole_mass = 0.25;

    // Distances are multiples of M, so changing the mass scales the hole without changing the view.
    // The camera must stay outside the photon sphere (3M).
    double orbit_radius_in_M = 26.0;
    double min_orbit_radius_in_M = 10.0;
    double max_orbit_radius_in_M = 200.0;
    double camera_elevation_degrees = 6.0; // Height above the disk plane
    double orbit_speed = 0.06;             // Automatic camera orbit, radians per second

    // Accretion disk. The inner edge sits at the innermost stable circular orbit (6M).
    double disk_inner_radius_in_M = 6.0;
    double disk_outer_radius_in_M = 20.0;
    double disk_peak_temperature = 5800.0; // Kelvin, in the gas's own frame

    // Simulated time per real second, in units of M (the light-crossing time of M).
    // The disk's inner edge takes about 92 M to complete one orbit.
    double time_scale_in_M = 20.0;

    double exposure = 0.6;

    double aspect_ratio() const {
        return static_cast<double>(screen_width) / screen_height;
    }
};
