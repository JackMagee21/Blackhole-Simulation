#pragma once

#include <SDL2/SDL.h>
#include <string>

// Input gathered during the last poll_events() call
struct InputState {
    int mouse_dx = 0;         // Mouse movement while the left button is held, in pixels
    int mouse_dy = 0;
    double wheel = 0.0;       // Scroll steps; positive scrolls away from the user
    std::string keys_pressed; // Characters of the keys pressed (letters, digits, space, symbols)
};

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
    void set_title(const std::string& title);

    const InputState& input() const { return m_input; }

    SDL_Renderer* get_renderer() const;

private:
    int m_width;
    int m_height;
    bool m_is_running;
    InputState m_input;

    // Raw SDL resources hidden from callers
    SDL_Window* m_sdl_window;
    SDL_Renderer* m_sdl_renderer;

    // Private helper: internal initialization logic
    bool init_sdl(const std::string& title);
};