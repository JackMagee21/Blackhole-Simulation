#pragma once

#include "Render/Colour.h"
#include <vector>

// Soft glow around very bright areas, imitating light scattering inside a camera lens or eye.
// Works on a quarter-resolution copy of the bright parts, so it costs very little.
class Bloom {
    public:
        // Adds the glow in place to a linear HDR image
        void apply(std::vector<Colour>& image, int width, int height, double threshold, double strength);

    private:
        std::vector<Colour> m_bright;
        std::vector<Colour> m_narrow;
        std::vector<Colour> m_wide;
        std::vector<Colour> m_temp;

        void blur(std::vector<Colour>& image, int width, int height, double sigma);
};
