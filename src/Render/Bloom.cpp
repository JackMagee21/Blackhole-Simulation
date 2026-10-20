#include "Bloom.h"
#include <algorithm>
#include <cmath>

namespace {
    const int downsample = 4;
}

// Separable Gaussian blur: one horizontal pass then one vertical pass
void Bloom::blur(std::vector<Colour>& image, int width, int height, double sigma) {
    int radius = static_cast<int>(std::ceil(3.0 * sigma));
    std::vector<double> kernel(2 * radius + 1);
    double total = 0.0;
    for (int i = -radius; i <= radius; ++i) {
        kernel[i + radius] = std::exp(-0.5 * i * i / (sigma * sigma));
        total += kernel[i + radius];
    }
    for (double& k : kernel) {
        k /= total;
    }

    m_temp.assign(image.size(), Colour::black());

    #pragma omp parallel for
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Colour sum;
            for (int i = -radius; i <= radius; ++i) {
                int sx = std::clamp(x + i, 0, width - 1);
                sum += image[y * width + sx] * kernel[i + radius];
            }
            m_temp[y * width + x] = sum;
        }
    }

    #pragma omp parallel for
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Colour sum;
            for (int i = -radius; i <= radius; ++i) {
                int sy = std::clamp(y + i, 0, height - 1);
                sum += m_temp[sy * width + x] * kernel[i + radius];
            }
            image[y * width + x] = sum;
        }
    }
}

void Bloom::apply(std::vector<Colour>& image, int width, int height, double threshold, double strength) {
    const int sw = (width + downsample - 1) / downsample;
    const int sh = (height + downsample - 1) / downsample;
    m_bright.assign(sw * sh, Colour::black());

    // 1. Average each block, keeping only the light above the threshold
    #pragma omp parallel for
    for (int sy = 0; sy < sh; ++sy) {
        for (int sx = 0; sx < sw; ++sx) {
            Colour sum;
            int count = 0;
            for (int y = sy * downsample; y < std::min((sy + 1) * downsample, height); ++y) {
                for (int x = sx * downsample; x < std::min((sx + 1) * downsample, width); ++x) {
                    const Colour& c = image[y * width + x];
                    double lum = c.luminance();
                    if (lum > threshold) {
                        sum += c * ((lum - threshold) / lum);
                    }
                    ++count;
                }
            }
            m_bright[sy * sw + sx] = sum * (1.0 / count);
        }
    }

    // 2. A tight glow plus a wide faint halo
    m_narrow = m_bright;
    m_wide = m_bright;
    blur(m_narrow, sw, sh, 1.5);
    blur(m_wide, sw, sh, 7.0);

    // 3. Upsample bilinearly and add back on top of the image
    #pragma omp parallel for
    for (int y = 0; y < height; ++y) {
        double fy = std::clamp((y + 0.5) / downsample - 0.5, 0.0, sh - 1.0);
        int y0 = static_cast<int>(fy);
        int y1 = std::min(y0 + 1, sh - 1);
        double ty = fy - y0;
        for (int x = 0; x < width; ++x) {
            double fx = std::clamp((x + 0.5) / downsample - 0.5, 0.0, sw - 1.0);
            int x0 = static_cast<int>(fx);
            int x1 = std::min(x0 + 1, sw - 1);
            double tx = fx - x0;

            auto at = [&](int ix, int iy) {
                int i = iy * sw + ix;
                return m_narrow[i] * 0.6 + m_wide[i] * 0.4;
            };
            Colour top = at(x0, y0) * (1.0 - tx) + at(x1, y0) * tx;
            Colour bottom = at(x0, y1) * (1.0 - tx) + at(x1, y1) * tx;
            image[y * width + x] += (top * (1.0 - ty) + bottom * ty) * strength;
        }
    }
}
