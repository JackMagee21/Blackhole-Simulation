#pragma once

#include "Math/Vec3.h"
#include "Math/Ray.h"
#include "Physics/Physics.h"
#include <vector>

// Precomputed photon deflection for a camera at a fixed distance from the black hole.
//
// Schwarzschild spacetime is spherically symmetric, so every photon stays in the plane
// containing its start point, its direction and the black hole. Its fate depends only on
// alpha, the angle between its initial direction and the direction to the hole. A few
// thousand rays are traced once per camera radius, and each pixel then costs a table
// lookup instead of a full geodesic integration.
//
// Assumes the camera is outside the photon sphere (r > 3M).
class DeflectionTable {
    public:
        explicit DeflectionTable(double M, int samples = 4096,
                                 const Physics::TracerSettings& tracer = Physics::TracerSettings());

        // Rebuilds the table only if the camera's distance from the hole has changed
        void update(double camera_radius);

        // Returns false if the ray falls into the black hole, otherwise writes its escape direction
        bool exit_direction(const Ray& ray, Vec3& out_dir) const;

    private:
        double m_M;
        int m_samples;
        Physics::TracerSettings m_tracer;
        double m_radius;
        double m_alpha_crit;              // Rays aimed closer than this to the hole are captured
        std::vector<double> m_exit_angle; // Unwrapped exit direction angle within the ray's plane

        double sample_alpha(int i) const;
        bool trace_in_plane(double alpha, double& exit_angle) const;
};
