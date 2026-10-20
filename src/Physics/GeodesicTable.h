#pragma once

#include "Math/Ray.h"
#include "Math/Vec3.h"
#include <vector>

// Precomputed photon paths for a static camera at a fixed distance from the black hole.
//
// Schwarzschild spacetime is spherically symmetric, so every photon stays in the plane
// containing the camera, its direction and the hole, and its path r(phi) in that plane
// depends only on alpha, the angle between its initial direction and the direction to the
// hole. The exact orbit equation d²u/dphi² = 3Mu² - u (u = 1/r) is integrated once for a
// few thousand alphas; each pixel then looks its path up instead of integrating it.
//
// Assumes the camera is outside the photon sphere (r > 3M).
class GeodesicTable {
    public:
        // One camera ray's path, located in the table
        struct Path {
            Vec3 x_axis;             // In-plane direction from the hole to the camera (phi = 0)
            Vec3 y_axis;             // In-plane direction of increasing phi
            double impact_parameter; // Photon angular momentum per unit energy
            double phi_end;          // Swept angle where the path escapes or hits the horizon
            bool escaped;
            int row;                 // Table row below this ray's alpha
            double weight;           // Interpolation weight toward row + 1
        };

        explicit GeodesicTable(double M, int rows_per_side = 2048);

        // Rebuilds the table only if the camera's distance from the hole has changed
        void update(double camera_radius);

        Path trace(const Ray& ray) const;

        // Distance from the hole at swept angle phi, or -1 if the path has already ended
        double radius_at(const Path& path, double phi) const;

        // Direction an escaped photon heads off to infinity in
        Vec3 escape_direction(const Path& path) const;

        double camera_radius() const { return m_radius; }

    private:
        // u = 1/r sampled every m_phi_step along one path, until it escapes or is captured
        struct Row {
            std::vector<float> u;
            double phi_end;
            double u_end; // 0 at escape, 1/r_horizon at capture
            bool escaped;
        };

        double m_M;
        int m_rows_per_side;
        double m_phi_step;
        double m_max_phi;     // Paths still circling the photon sphere after this count as trapped
        double m_radius;
        double m_alpha_crit;  // Rays aimed closer than this to the hole are captured
        std::vector<Row> m_rows; // Captured side then escaping side, alpha ascending

        double row_alpha(int i) const;
        void locate(double alpha, int& row, double& weight) const;
        Row integrate_row(double alpha) const;
        double row_u(const Row& row, double phi) const;
};
