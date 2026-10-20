#include "Starfield.h"
#include "Math/Noise.h"
#include "Render/Blackbody.h"
#include <algorithm>
#include <cmath>

namespace {

    // The cube face a direction points through, and its coordinates on that face in [-1, 1]
    void to_cube_face(const Vec3& d, int& face, double& u, double& v) {
        double ax = std::abs(d.x);
        double ay = std::abs(d.y);
        double az = std::abs(d.z);
        if (ax >= ay && ax >= az) {
            face = d.x > 0.0 ? 0 : 1;
            u = d.y / ax;
            v = d.z / ax;
        } else if (ay >= az) {
            face = d.y > 0.0 ? 2 : 3;
            u = d.x / ay;
            v = d.z / ay;
        } else {
            face = d.z > 0.0 ? 4 : 5;
            u = d.x / az;
            v = d.y / az;
        }
    }

    Vec3 from_cube_face(int face, double u, double v) {
        switch (face) {
            case 0:  return Vec3(1.0, u, v);
            case 1:  return Vec3(-1.0, u, v);
            case 2:  return Vec3(u, 1.0, v);
            case 3:  return Vec3(u, -1.0, v);
            case 4:  return Vec3(u, v, 1.0);
            default: return Vec3(u, v, -1.0);
        }
    }

} // namespace

Starfield::Starfield(double star_size)
    : m_star_size(star_size),
      m_band_normal(Vec3(0.35, 1.0, -0.45).normalized())
{
}

// Each cube-face cell holds at most one star, placed away from the cell's edges so
// its glow never gets cut off at a border. Only the cell under dir needs checking.
Colour Starfield::stars(const Vec3& dir, int cells, double probability, double brightness, int layer) const {
    int face;
    double u, v;
    to_cube_face(dir, face, u, v);

    int iu = std::min(static_cast<int>((u + 1.0) * 0.5 * cells), cells - 1);
    int iv = std::min(static_cast<int>((v + 1.0) * 0.5 * cells), cells - 1);

    uint32_t h = Noise::hash(iu, iv, face + 8 * layer);
    if (Noise::to_unit(h) > probability) {
        return Colour::black();
    }

    const double margin = 0.3;
    double su = iu + margin + (1.0 - 2.0 * margin) * Noise::to_unit(Noise::hash(h + 1));
    double sv = iv + margin + (1.0 - 2.0 * margin) * Noise::to_unit(Noise::hash(h + 2));
    Vec3 star = from_cube_face(face, su / cells * 2.0 - 1.0, sv / cells * 2.0 - 1.0).normalized();

    double angle2 = (dir - star).norm_squared();
    double falloff = std::exp(-angle2 / (2.0 * m_star_size * m_star_size));
    if (falloff < 1e-4) {
        return Colour::black();
    }

    // Most stars are faint, a few are bright; colours range from red dwarfs to blue giants
    double magnitude = std::pow(Noise::to_unit(Noise::hash(h + 3)), 6.0);
    double kelvin = 2800.0 + 12000.0 * std::pow(Noise::to_unit(Noise::hash(h + 4)), 2.0);
    Colour tint = Blackbody::radiance(kelvin);
    tint = tint * (1.0 / std::max(tint.luminance(), 1e-9));

    return tint * (brightness * (0.08 + magnitude) * falloff);
}

// Diffuse glow of unresolved stars along a great circle, broken up by dark dust lanes
Colour Starfield::galaxy(const Vec3& dir) const {
    double height = dir.dot(m_band_normal);
    double band = std::exp(-height * height / (2.0 * 0.13 * 0.13));
    if (band < 0.01) {
        return Colour::black();
    }

    double clouds = Noise::fbm(dir.x * 4.0 + 11.0, dir.y * 4.0, dir.z * 4.0, 5);
    double dust = Noise::fbm(dir.x * 7.0, dir.y * 7.0 + 5.0, dir.z * 7.0, 4);
    double intensity = band * clouds * (1.0 - 0.8 * std::clamp((dust - 0.45) * 4.0, 0.0, 1.0));

    return Colour(1.0, 0.9, 0.78) * (0.05 * intensity);
}

Colour Starfield::sample(const Vec3& dir) const {
    Colour c = galaxy(dir);
    c += stars(dir, 90, 0.55, 0.6, 0);
    c += stars(dir, 40, 0.35, 2.0, 1);
    c += stars(dir, 14, 0.30, 8.0, 2);
    return c;
}
