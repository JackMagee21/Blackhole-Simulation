#pragma once

#include <cmath>

// Closed-form results for a non-rotating (Schwarzschild) black hole of mass M,
// in geometric units (G = c = 1), so every length and time is a multiple of M.
namespace Schwarzschild {

    // Event horizon: nothing inside escapes
    inline double horizon_radius(double M) { return 2.0 * M; }

    // Light can orbit here, unstably; it sets the edge of the black hole's shadow
    inline double photon_sphere_radius(double M) { return 3.0 * M; }

    // Innermost stable circular orbit: gas inside this plunges in, so disks end here
    inline double isco_radius(double M) { return 6.0 * M; }

    // Photons with a smaller impact parameter are captured
    inline double critical_impact_parameter(double M) { return 3.0 * std::sqrt(3.0) * M; }

    // Photon orbit equation in u = 1/r against swept angle phi: d²u/dphi² = 3 M u² - u
    inline double photon_orbit_acceleration(double u, double M) {
        return 3.0 * M * u * u - u;
    }

    // Angular velocity of gas on a circular orbit, as seen from far away
    inline double orbital_angular_velocity(double r, double M) {
        return std::sqrt(M / (r * r * r));
    }

    // Frequency shift g = observed / emitted for light from gas on a circular orbit at radius r
    // (orbiting in the +z sense), received by a static observer at r_observer.
    // lambda_z is the photon's angular momentum about the orbit axis per unit energy.
    // Combines gravitational redshift, time dilation and the Doppler shift of the moving gas.
    inline double disk_frequency_shift(double r, double lambda_z, double r_observer, double M) {
        double omega = orbital_angular_velocity(r, M);
        return std::sqrt(1.0 - 3.0 * M / r)
             / (std::sqrt(1.0 - 2.0 * M / r_observer) * (1.0 - omega * lambda_z));
    }

} // namespace Schwarzschild
