#include "PixelBuffer.h"
#include <iostream>

PixelBuffer::PixelBuffer(SDL_Renderer* renderer, int width, int height)
    : m_width(width), m_height(height), m_renderer(renderer), m_texture(nullptr)
{
    m_pixels.resize(width * height, 0xFF000000);

    m_texture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        m_width,
        m_height
    );

    if (!m_texture) {
        std::cerr << "Failed to create SDL_Texture: " << SDL_GetError() << "\n";
    }
}

PixelBuffer::~PixelBuffer() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
    }
}

void PixelBuffer::set_pixel(int x, int y, const Color& color) {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) return;
    m_pixels[y * m_width + x] = color.to_argb32();
}

void PixelBuffer::clear(const Color& color) {
    uint32_t packed = color.to_argb32();
    std::fill(m_pixels.begin(), m_pixels.end(), packed);
}

void PixelBuffer::render() {
    SDL_UpdateTexture(m_texture, nullptr, m_pixels.data(), m_width * sizeof(uint32_t));
    SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
}