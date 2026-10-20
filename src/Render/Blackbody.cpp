#include "Blackbody.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

    const double min_kelvin = 300.0;
    const double max_kelvin = 200000.0;
    const int table_size = 2048;

    // Asymmetric Gaussian lobe used by the CIE fit below
    double lobe(double x, double mu, double sigma_low, double sigma_high) {
        double t = (x - mu) / (x < mu ? sigma_low : sigma_high);
        return std::exp(-0.5 * t * t);
    }

    // CIE 1931 colour matching functions, multi-lobe fit by Wyman, Sloan & Shirley (2013)
    void cie_xyz(double nm, double& X, double& Y, double& Z) {
        X = 1.056 * lobe(nm, 599.8, 37.9, 31.0) + 0.362 * lobe(nm, 442.0, 16.0, 26.7)
          - 0.065 * lobe(nm, 501.1, 20.4, 26.2);
        Y = 0.821 * lobe(nm, 568.8, 46.9, 40.5) + 0.286 * lobe(nm, 530.9, 16.3, 31.1);
        Z = 1.217 * lobe(nm, 437.0, 11.8, 36.0) + 0.681 * lobe(nm, 459.0, 26.0, 13.8);
    }

    // Planck's law, up to a constant factor
    double planck(double nm, double kelvin) {
        const double c2 = 1.4388e-2; // Second radiation constant hc/k, in metre kelvin
        double metres = nm * 1e-9;
        return 1.0 / (std::pow(metres, 5) * std::expm1(c2 / (metres * kelvin)));
    }

    Colour integrate(double kelvin) {
        double X = 0.0, Y = 0.0, Z = 0.0;
        for (double nm = 380.0; nm <= 780.0; nm += 2.0) {
            double x, y, z;
            cie_xyz(nm, x, y, z);
            double power = planck(nm, kelvin);
            X += x * power;
            Y += y * power;
            Z += z * power;
        }

        // CIE XYZ to linear sRGB. Very hot or cool colours fall slightly outside the sRGB
        // gamut and come out with a small negative channel, which is clipped.
        return {
            std::max(0.0,  3.2406 * X - 1.5372 * Y - 0.4986 * Z),
            std::max(0.0, -0.9689 * X + 1.8758 * Y + 0.0415 * Z),
            std::max(0.0,  0.0557 * X - 0.2040 * Y + 1.0570 * Z)
        };
    }

    // Log-spaced lookup table, built once on first use
    struct Table {
        std::vector<Colour> entries;

        Table() : entries(table_size) {
            for (int i = 0; i < table_size; ++i) {
                entries[i] = integrate(kelvin_at(i));
            }
            double scale = 1.0 / integrate(6500.0).luminance();
            for (Colour& c : entries) {
                c = c * scale;
            }
        }

        static double kelvin_at(double index) {
            double t = index / (table_size - 1);
            return min_kelvin * std::pow(max_kelvin / min_kelvin, t);
        }
    };

    const Table& table() {
        static const Table instance;
        return instance;
    }

} // namespace

Colour Blackbody::radiance(double kelvin) {
    const Table& t = table();
    kelvin = std::clamp(kelvin, min_kelvin, max_kelvin);

    double x = std::log(kelvin / min_kelvin) / std::log(max_kelvin / min_kelvin) * (table_size - 1);
    int i = std::min(static_cast<int>(x), table_size - 2);
    double f = x - i;
    return t.entries[i] * (1.0 - f) + t.entries[i + 1] * f;
}
