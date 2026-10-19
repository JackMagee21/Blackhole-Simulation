#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>
#include "Colour.h"

class PixelBuffer {
public:
    PixelBuffer(SDL_Renderer* renderer, int width, int height);
    ~PixelBuffer();

    PixelBuffer(const PixelBuffer&) = delete;
    PixelBuffer& operator=(const PixelBuffer&) = delete;

    // Set pixel using our strong Color type
    void set_pixel(int x, int y, const Colour& color);

    // Clear entire buffer with a background color
    void clear(const Colour& color = Colour::black());

    // Stream CPU buffer to the GPU SDL_Texture
    void render();

    int get_width() const { return m_width; }
    int get_height() const { return m_height; }

private:
    int m_width;
    int m_height;
    SDL_Renderer* m_renderer;
    SDL_Texture*  m_texture;
    std::vector<uint32_t> m_pixels; // Flat ARGB8888 array
};