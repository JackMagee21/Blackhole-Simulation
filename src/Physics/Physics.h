#pragma once

#include "Vec3/Vec3.h"
#include <cmath>

namespace Physics {

    // Evaluates the spatial acceleration of a photon in Schwarzschild spacetime.
    //
    // Derived from the null geodesic equation (ds^2 = 0) in isotropic/Cartesian 
    // coordinates, the effective bending acceleration toward mass M at origin is:
    //   a = -3 * M * |h|^2 / r^5 * r_hat
    // where h = x cross v is specific orbital angular momentum.
    inline Vec3 geodesic_acceleration(const Vec3& pos, const Vec3& vel, double M = 1.0) {
        double r2 = pos.norm_squared();
        double r = std::sqrt(r2);

        // Singularity safeguard
        if (r < 1e-4) {
            return Vec3(0.0, 0.0, 0.0);
        }

        Vec3 h = pos.cross(vel);
        double h2 = h.norm_squared();

        // a = pos * (-3 * M * h^2 / r^6)
        double r6 = r2 * r2 * r2;
        double factor = -3.0 * M * h2 / r6;

        return pos * factor;
    }

    // Advances photon position and direction forward by step dt using RK4.
    inline void step_rk4(Vec3& pos, Vec3& vel, double dt, double M = 1.0) {
        // Stage 1
        Vec3 k1_v = geodesic_acceleration(pos, vel, M);
        Vec3 k1_x = vel;

        // Stage 2
        Vec3 p2 = pos + k1_x * (dt * 0.5);
        Vec3 v2 = vel + k1_v * (dt * 0.5);
        Vec3 k2_v = geodesic_acceleration(p2, v2, M);
        Vec3 k2_x = v2;

        // Stage 3
        Vec3 p3 = pos + k2_x * (dt * 0.5);
        Vec3 v3 = vel + k2_v * (dt * 0.5);
        Vec3 k3_v = geodesic_acceleration(p3, v3, M);
        Vec3 k3_x = v3;

        // Stage 4
        Vec3 p4 = pos + k3_x * dt;
        Vec3 v4 = vel + k3_v * dt;
        Vec3 k4_v = geodesic_acceleration(p4, v4, M);
        Vec3 k4_x = v4;

        // Weighted integration update
        pos = pos + (k1_x + k2_x * 2.0 + k3_x * 2.0 + k4_x) * (dt / 6.0);
        vel = vel + (k1_v + k2_v * 2.0 + k3_v * 2.0 + k4_v) * (dt / 6.0);

        // Light in natural units always moves at speed c = 1.0
        vel = vel.normalized();
    }

} // namespace Physics