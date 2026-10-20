#pragma once

#include "Render/Colour.h"
#include <vector>

// A geometrically thin accretion disk in the y = 0 plane, orbiting the hole anticlockwise
// seen from above (+y). Gas moves on circular Keplerian orbits and glows as a blackbody
// with the standard thin-disk (Shakura-Sunyaev / Novikov-Thorne) temperature profile,
// which is zero at the inner edge, peaks just outside it and falls off as r^(-3/4).
class AccretionDisk {
    public:
        struct Sample {
            Colour emission; // Light reaching the camera, already weighted by opacity
            double opacity;  // Fraction of light from behind that this point blocks
        };

        AccretionDisk(double M, double inner_radius, double outer_radius, double peak_temperature);

        bool contains(double r) const { return r >= m_inner && r <= m_outer; }

        // Gas temperature at radius r, in its own rest frame
        double temperature(double r) const;

        // How the disk looks at radius r and disk-plane angle at a simulation time, for light
        // whose frequency is shifted by g = observed / emitted on its way to the camera
        Sample sample(double r, double angle, double time, double g) const;

    private:
        double m_M;
        double m_inner;
        double m_outer;
        double m_peak_temperature;
        double m_temperature_scale;
        double m_brightness_scale;
        double m_flow_period;

        // The turbulence pattern never changes in the gas's own coordinates (time only turns
        // it), so each layer is precomputed once as a polar texture over radius and angle
        static const int pattern_layers = 2;
        int m_radial_texels;
        int m_angular_texels;
        std::vector<float> m_patterns[pattern_layers];

        double turbulence(double r, double angle, double time) const;
        double compute_pattern(double r, double angle, int layer) const;
        double pattern(double r, double angle, int layer) const;
};
