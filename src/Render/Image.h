#pragma once

#include "Render/Colour.h"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

// Plain CPU image with the same pixel interface as PixelBuffer, for rendering without a window
class Image {
    public:
        Image(int width, int height) : m_width(width), m_height(height), m_pixels(width * height) {}

        int get_width() const { return m_width; }
        int get_height() const { return m_height; }

        void set_pixel(int x, int y, const Colour& colour) {
            m_pixels[y * m_width + x] = colour.to_argb32();
        }

        // Writes a binary PPM, which most image viewers and converters can open
        bool save_ppm(const std::string& path) const {
            std::ofstream file(path, std::ios::binary);
            if (!file) {
                return false;
            }
            file << "P6\n" << m_width << " " << m_height << "\n255\n";
            for (uint32_t p : m_pixels) {
                char rgb[3] = {
                    static_cast<char>((p >> 16) & 0xFF),
                    static_cast<char>((p >> 8) & 0xFF),
                    static_cast<char>(p & 0xFF)
                };
                file.write(rgb, 3);
            }
            return static_cast<bool>(file);
        }

    private:
        int m_width;
        int m_height;
        std::vector<uint32_t> m_pixels;
};
