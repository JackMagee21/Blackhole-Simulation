#pragma once

#include "Render/Colour.h"

// Light given off by a blackbody (an idealised hot glowing object) at a temperature in kelvin,
// as linear sRGB. Computed by integrating Planck's law against the CIE 1931 colour matching
// functions, so both the hue and the relative brightness are physical: a 3000 K body is dim
// and orange, a 10000 K body far brighter and blue-white. Normalised so 6500 K has luminance 1.
namespace Blackbody {

    Colour radiance(double kelvin);

} // namespace Blackbody
