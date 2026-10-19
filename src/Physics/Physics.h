#pragma once

#include "Math/Vec3.h"
#include <algorithm>
#include <cmath>

namespace Physics {

    // Evaluates the spatial acceleration of a photon in Schwarzschild spacetime.
    //
    // The null geodesic orbit equation u'' + u = 3 * M * u^2 (with u = 1/r) is reproduced
    // exactly by the central bending acceleration toward mass M at origin:
    //   a = -3 * M * |h|^2 / r^4 * r_hat = -3 * M * |h|^2 * pos / r^5
    // where h = x cross v is specific orbital angular momentum. Since this is a central
    // force, |h|^2 is conserved along the ray, so it is computed once and passed in.
    inline Vec3 geodesic_acceleration(const Vec3& pos, double h2, double M) {
        double r2 = pos.norm_squared();

        // Singularity safeguard
        if (r2 < 1e-8 * M * M) {
            return Vec3(0.0, 0.0, 0.0);
        }

        double r5 = r2 * r2 * std::sqrt(r2);
        return pos * (-3.0 * M * h2 / r5);
    }

    // Advances photon position and velocity forward by step dt using RK4.
    //
    // vel is deliberately not renormalised afterwards: that would change h and distort the
    // orbit. Only the shape of the path matters here, not the speed along it.
    inline void step_rk4(Vec3& pos, Vec3& vel, double dt, double h2, double M) {
        // Stage 1
        Vec3 k1_v = geodesic_acceleration(pos, h2, M);
        Vec3 k1_x = vel;

        // Stage 2
        Vec3 p2 = pos + k1_x * (dt * 0.5);
        Vec3 v2 = vel + k1_v * (dt * 0.5);
        Vec3 k2_v = geodesic_acceleration(p2, h2, M);
        Vec3 k2_x = v2;

        // Stage 3
        Vec3 p3 = pos + k2_x * (dt * 0.5);
        Vec3 v3 = vel + k2_v * (dt * 0.5);
        Vec3 k3_v = geodesic_acceleration(p3, h2, M);
        Vec3 k3_x = v3;

        // Stage 4
        Vec3 p4 = pos + k3_x * dt;
        Vec3 v4 = vel + k3_v * dt;
        Vec3 k4_v = geodesic_acceleration(p4, h2, M);
        Vec3 k4_x = v4;

        // Weighted integration update
        pos = pos + (k1_x + k2_x * 2.0 + k3_x * 2.0 + k4_x) * (dt / 6.0);
        vel = vel + (k1_v + k2_v * 2.0 + k3_v * 2.0 + k4_v) * (dt / 6.0);
    }

    // Limits and accuracy for trace_photon. Lengths are in units of M so the same settings
    // give the same accuracy for any black hole mass.
    struct TracerSettings {
        double escape_radius_in_M = 1000.0; // Far field boundary (remaining bending negligible)
        int max_steps = 20000;               // Beyond this the photon counts as trapped near r = 3M
        double step_scale = 0.03;            // Step size relative to distance, far from the hole
        double min_step_in_M = 0.02;         // Smallest step, used right next to the horizon
    };

    // Step size grows with distance: small near the photon sphere where rays bend sharply,
    // large far away where spacetime is nearly flat and a fixed tiny step is wasted work.
    inline double adaptive_step(double r, double M, const TracerSettings& settings) {
        double x = r / M;
        return M * std::max(settings.step_scale * x * x / (1.0 + x), settings.min_step_in_M);
    }

    struct PhotonResult {
        bool captured;
        Vec3 exit_dir; // Unit escape direction (only valid when not captured)
    };

    // Marches a single photon through curved Schwarzschild spacetime until it either
    // crosses the event horizon or escapes to effectively flat space.
    inline PhotonResult trace_photon(Vec3 pos, Vec3 vel, double M,
                                     const TracerSettings& settings = TracerSettings()) {
        const double r_horizon = 2.0 * M;   // Event horizon boundary (Schwarzschild radius)
        const double r_escape = settings.escape_radius_in_M * M;

        const double r_horizon2 = r_horizon * r_horizon;
        const double r_escape2 = r_escape * r_escape;
        const double h2 = pos.cross(vel).norm_squared();

        for (int step = 0; step < settings.max_steps; ++step) {
            double r2 = pos.norm_squared();

            // 1. Ray fell into the event horizon -> Swallowed forever
            if (r2 <= r_horizon2) {
                return {true, Vec3()};
            }

            // 2. Ray escaped to deep space
            if (r2 >= r_escape2) {
                return {false, vel.normalized()};
            }

            // 3. Step forward along null geodesic
            step_rk4(pos, vel, adaptive_step(std::sqrt(r2), M, settings), h2, M);
        }

        // Trapped in unstable orbits near photon sphere (r = 3M)
        return {true, Vec3()};
    }

} // namespace Physics
