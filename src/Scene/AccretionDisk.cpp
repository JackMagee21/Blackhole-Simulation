#include "AccretionDisk.h"
#include "Math/Noise.h"
#include "Physics/Schwarzschild.h"
#include "Render/Blackbody.h"
#include <algorithm>
#include <cmath>

namespace {

    // Shape of the thin-disk temperature profile, up to a constant
    double temperature_profile(double r, double inner) {
        return std::pow(std::max(0.0, (1.0 - std::sqrt(inner / r))) / (r * r * r), 0.25);
    }

    double smoothstep(double edge0, double edge1, double x) {
        double t = std::clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
        return t * t * (3.0 - 2.0 * t);
    }

} // namespace

AccretionDisk::AccretionDisk(double M, double inner_radius, double outer_radius, double peak_temperature)
    : m_M(M),
      m_inner(inner_radius),
      m_outer(outer_radius),
      m_peak_temperature(peak_temperature)
{
    // The profile peaks at r = (49/36) r_inner
    m_temperature_scale = 1.0 / temperature_profile(49.0 / 36.0 * m_inner, m_inner);

    // Hottest gas, seen without any frequency shift, has brightness 1
    m_brightness_scale = 1.0 / Blackbody::radiance(m_peak_temperature).luminance();

    // Two orbits of the inner edge; see turbulence()
    m_flow_period = 2.0 * 2.0 * M_PI / Schwarzschild::orbital_angular_velocity(m_inner, m_M);

    m_radial_texels = 1024;
    m_angular_texels = 2048;
    for (int layer = 0; layer < pattern_layers; ++layer) {
        std::vector<float>& texture = m_patterns[layer];
        texture.resize(m_radial_texels * m_angular_texels);

        #pragma omp parallel for
        for (int i = 0; i < m_radial_texels; ++i) {
            double r = m_inner + (m_outer - m_inner) * i / (m_radial_texels - 1);
            for (int j = 0; j < m_angular_texels; ++j) {
                double angle = 2.0 * M_PI * j / m_angular_texels;
                texture[i * m_angular_texels + j] = static_cast<float>(compute_pattern(r, angle, layer));
            }
        }
    }
}

double AccretionDisk::temperature(double r) const {
    return m_peak_temperature * m_temperature_scale * temperature_profile(r, m_inner);
}

// Streaky turbulence pattern, stretched along the orbit direction like sheared gas.
// Sampling on a circle keeps the pattern seamless all the way around.
double AccretionDisk::compute_pattern(double r, double angle, int layer) const {
    double x = r / m_M;
    double offset = 37.0 * layer;

    // Large clumps, then thinner filaments stretched along the orbit
    double clumps = Noise::fbm(std::cos(angle) * 1.6 + offset, std::sin(angle) * 1.6, x * 0.45, 4);
    double filaments = Noise::fbm(std::cos(angle) * 4.0, std::sin(angle) * 4.0 + offset, x * 1.8, 4);
    return std::clamp(0.6 * clumps + 0.4 * filaments, 0.0, 1.0);
}

// Bilinear lookup into a precomputed layer, wrapping around in angle
double AccretionDisk::pattern(double r, double angle, int layer) const {
    const std::vector<float>& texture = m_patterns[layer];

    double fi = std::clamp((r - m_inner) / (m_outer - m_inner), 0.0, 1.0) * (m_radial_texels - 1);
    int i0 = std::min(static_cast<int>(fi), m_radial_texels - 2);
    double ti = fi - i0;

    double turns = angle / (2.0 * M_PI);
    double fj = (turns - std::floor(turns)) * m_angular_texels;
    int j0 = static_cast<int>(fj) % m_angular_texels;
    int j1 = (j0 + 1) % m_angular_texels;
    double tj = fj - std::floor(fj);

    const float* row0 = &texture[i0 * m_angular_texels];
    const float* row1 = row0 + m_angular_texels;
    double a = row0[j0] + (row0[j1] - row0[j0]) * tj;
    double b = row1[j0] + (row1[j1] - row1[j0]) * tj;
    return a + (b - a) * ti;
}

// Gas at radius r orbits at its own angular velocity, so any pattern gets wound into
// ever-tighter spirals. Blending two copies of the pattern that restart half a period
// apart keeps that shear bounded while each part of the disk still visibly moves at its
// correct orbital speed.
double AccretionDisk::turbulence(double r, double angle, double time) const {
    const double golden_angle = 2.39996;
    double omega = Schwarzschild::orbital_angular_velocity(r, m_M);

    double cycle_a = std::floor(time / m_flow_period);
    double cycle_b = std::floor(time / m_flow_period + 0.5);
    double t_a = time - cycle_a * m_flow_period;
    double t_b = time - (cycle_b - 0.5) * m_flow_period;
    double w_a = 1.0 - std::abs(2.0 * t_a / m_flow_period - 1.0);

    // Gas moves toward decreasing angle, so look up where it started from. Each time a
    // layer restarts (while its weight is zero) it jumps to a new part of the pattern,
    // so the animation never repeats.
    return w_a * pattern(r, angle + omega * t_a + golden_angle * cycle_a, 0)
         + (1.0 - w_a) * pattern(r, angle + omega * t_b + golden_angle * cycle_b, 1);
}

AccretionDisk::Sample AccretionDisk::sample(double r, double angle, double time, double g) const {
    // Soft outer edge; the inner edge at the ISCO is physically sharp
    double edge = smoothstep(m_outer, 0.7 * m_outer, r) * smoothstep(m_inner, m_inner + 0.15 * m_M, r);

    double density = smoothstep(0.3, 0.7, turbulence(r, angle, time));
    double opacity = edge * (0.6 + 0.4 * density);

    // Light keeps a blackbody spectrum when its frequency shifts, just at temperature g T,
    // which makes approaching gas bluer and far brighter, receding gas redder and dimmer
    Colour light = Blackbody::radiance(g * temperature(r)) * m_brightness_scale;

    return {light * (opacity * (0.25 + 1.1 * density)), opacity};
}
