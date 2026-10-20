#include "GeodesicTable.h"
#include "Physics/Schwarzschild.h"
#include <algorithm>
#include <cmath>

namespace {

    // The photon orbit equation as a first-order system: u' = w, w' = 3Mu² - u
    struct OrbitState {
        double u;
        double w;
    };

    OrbitState rk4_step(const OrbitState& s, double h, double M) {
        auto derivative = [M](double u, double w) {
            return OrbitState{w, Schwarzschild::photon_orbit_acceleration(u, M)};
        };

        OrbitState k1 = derivative(s.u, s.w);
        OrbitState k2 = derivative(s.u + 0.5 * h * k1.u, s.w + 0.5 * h * k1.w);
        OrbitState k3 = derivative(s.u + 0.5 * h * k2.u, s.w + 0.5 * h * k2.w);
        OrbitState k4 = derivative(s.u + h * k3.u, s.w + h * k3.w);

        return {
            s.u + (h / 6.0) * (k1.u + 2.0 * k2.u + 2.0 * k3.u + k4.u),
            s.w + (h / 6.0) * (k1.w + 2.0 * k2.w + 2.0 * k3.w + k4.w)
        };
    }

} // namespace

GeodesicTable::GeodesicTable(double M, int rows_per_side)
    : m_M(M),
      m_rows_per_side(rows_per_side),
      m_phi_step(M_PI / 512.0),
      m_max_phi(6.0 * M_PI),
      m_radius(-1.0),
      m_alpha_crit(0.0)
{
}

// Rows are packed quadratically toward alpha_crit from both sides, where rays wind
// around the photon sphere and their paths change fastest
double GeodesicTable::row_alpha(int i) const {
    const int n = m_rows_per_side;
    if (i < n) {
        double s = static_cast<double>(n - i) / n;
        return m_alpha_crit * (1.0 - s * s);
    }
    double t = static_cast<double>(i - n + 1) / n;
    return m_alpha_crit + (M_PI - m_alpha_crit) * t * t;
}

// Inverse of row_alpha(): the row just below alpha and how far toward the next one it is.
// Never blends across the shadow edge, where captured and escaping paths differ completely.
void GeodesicTable::locate(double alpha, int& row, double& weight) const {
    const int n = m_rows_per_side;
    if (alpha < m_alpha_crit) {
        double k = n * (1.0 - std::sqrt(1.0 - alpha / m_alpha_crit));
        k = std::clamp(k, 0.0, static_cast<double>(n - 1));
        row = static_cast<int>(k);
        weight = (row == n - 1) ? 0.0 : k - row;
        return;
    }
    double k = n * std::sqrt((alpha - m_alpha_crit) / (M_PI - m_alpha_crit)) - 1.0;
    k = std::clamp(k, 0.0, static_cast<double>(n - 1));
    int j = std::min(static_cast<int>(k), n - 2);
    row = n + j;
    weight = k - j;
}

GeodesicTable::Row GeodesicTable::integrate_row(double alpha) const {
    const double u_horizon = 1.0 / Schwarzschild::horizon_radius(m_M);
    const double u0 = 1.0 / m_radius;

    // A static camera measures alpha in its own local frame, so the photon starts with
    // du/dphi = u sqrt(1 - 2M/r) cot(alpha), which includes gravitational aberration
    const double sin_alpha = std::max(std::sin(alpha), 1e-9);
    OrbitState state{u0, u0 * std::sqrt(1.0 - 2.0 * m_M * u0) * std::cos(alpha) / sin_alpha};

    Row row;
    row.u.push_back(static_cast<float>(u0));
    double phi = 0.0;

    while (true) {
        // Advance to the next stored sample in sub-steps that change u by only a few percent,
        // which matters for nearly radial rays where u changes steeply with phi
        double remaining = m_phi_step;
        while (remaining > 0.0) {
            double h = remaining;
            if (state.w != 0.0) {
                h = std::min(h, 0.05 * std::max(state.u, 0.05 * u0) / std::abs(state.w));
            }
            OrbitState next = rk4_step(state, h, m_M);

            // Escaped: u reached 0 (r = infinity)
            if (next.u <= 0.0) {
                row.escaped = true;
                row.u_end = 0.0;
                row.phi_end = phi + h * state.u / (state.u - next.u);
                return row;
            }

            // Captured: crossed the event horizon
            if (next.u >= u_horizon) {
                row.escaped = false;
                row.u_end = u_horizon;
                row.phi_end = phi + h * (u_horizon - state.u) / (next.u - state.u);
                return row;
            }

            state = next;
            phi += h;
            remaining -= h;
        }

        row.u.push_back(static_cast<float>(state.u));

        // Still circling the photon sphere: treat as captured
        if (phi > m_max_phi) {
            row.escaped = false;
            row.u_end = state.u;
            row.phi_end = phi;
            return row;
        }
    }
}

void GeodesicTable::update(double camera_radius) {
    if (std::abs(camera_radius - m_radius) < 1e-9) {
        return;
    }
    m_radius = camera_radius;

    // Shadow edge as measured by a static camera: sin(alpha) = b_crit sqrt(1 - 2M/r) / r
    double sin_crit = Schwarzschild::critical_impact_parameter(m_M)
                    * std::sqrt(1.0 - 2.0 * m_M / m_radius) / m_radius;
    m_alpha_crit = std::asin(std::min(sin_crit, 1.0));

    m_rows.resize(2 * m_rows_per_side);
    #pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < 2 * m_rows_per_side; ++i) {
        m_rows[i] = integrate_row(row_alpha(i));
    }
}

GeodesicTable::Path GeodesicTable::trace(const Ray& ray) const {
    Path path;
    path.x_axis = ray.origin.normalized();

    // alpha is measured from the direction toward the hole
    double cos_alpha = std::clamp(-ray.direction.dot(path.x_axis), -1.0, 1.0);
    double alpha = std::acos(cos_alpha);

    Vec3 perp = ray.direction + path.x_axis * cos_alpha;
    double perp_len2 = perp.norm_squared();
    if (perp_len2 > 1e-24) {
        path.y_axis = perp / std::sqrt(perp_len2);
    } else {
        // Radial ray: any plane through the hole contains it
        Vec3 helper = std::abs(path.x_axis.y) < 0.9 ? Vec3(0.0, 1.0, 0.0) : Vec3(1.0, 0.0, 0.0);
        path.y_axis = path.x_axis.cross(helper).normalized();
    }

    path.impact_parameter = m_radius * std::sin(alpha) / std::sqrt(1.0 - 2.0 * m_M / m_radius);

    locate(alpha, path.row, path.weight);
    const Row& lower = m_rows[path.row];
    if (path.weight > 0.0) {
        const Row& upper = m_rows[path.row + 1];
        path.phi_end = lower.phi_end + (upper.phi_end - lower.phi_end) * path.weight;
        path.escaped = path.weight < 0.5 ? lower.escaped : upper.escaped;
    } else {
        path.phi_end = lower.phi_end;
        path.escaped = lower.escaped;
    }
    return path;
}

double GeodesicTable::row_u(const Row& row, double phi) const {
    if (phi >= row.phi_end) {
        return row.u_end;
    }

    double x = phi / m_phi_step;
    int j = static_cast<int>(x);
    int last = static_cast<int>(row.u.size()) - 1;

    // Final partial segment between the last stored sample and the end point
    if (j >= last) {
        double phi_last = last * m_phi_step;
        double t = (phi - phi_last) / (row.phi_end - phi_last);
        return row.u[last] + (row.u_end - row.u[last]) * t;
    }

    double t = x - j;
    return row.u[j] + (row.u[j + 1] - row.u[j]) * t;
}

double GeodesicTable::radius_at(const Path& path, double phi) const {
    if (phi >= path.phi_end) {
        return -1.0;
    }

    double u = row_u(m_rows[path.row], phi);
    if (path.weight > 0.0) {
        u += (row_u(m_rows[path.row + 1], phi) - u) * path.weight;
    }
    return u > 0.0 ? 1.0 / u : -1.0;
}

Vec3 GeodesicTable::escape_direction(const Path& path) const {
    return path.x_axis * std::cos(path.phi_end) + path.y_axis * std::sin(path.phi_end);
}
