#pragma once

#include <SDL2/SDL.h>
#include <string>

class Window {
public:
    // Lifecycle: Constructor & Destructor
    Window(const std::string& title, int width, int height);
    ~Window();

    // Prevent accidental copying (raw pointer double-free prevention)
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Simulation loop operations
    bool is_open() const;
    void poll_events();
    void clear(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0);
    void present();

    // Getters for dimensions (useful for ray-tracing resolution)
    int get_width() const;
    int get_height() const;

private:
    int m_width;
    int m_height;
    bool m_is_running;

    // Raw SDL resources hidden from callers
    SDL_Window* m_sdl_window;
    SDL_Renderer* m_sdl_renderer;

    // Private helper: internal initialization logic
    bool init_sdl(const std::string& title);
};