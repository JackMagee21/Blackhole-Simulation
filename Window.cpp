#include "Window.h"
#include <iostream>

Window::Window(const std::string& title, int width, int height)
    : m_width(width), m_height(height), m_is_running(false),
      m_sdl_window(nullptr), m_sdl_renderer(nullptr) 
{
    m_is_running = init_sdl(title);
}

Window::~Window() {
    // RAII: Cleaning up internal SDL state automatically
    if (m_sdl_renderer) SDL_DestroyRenderer(m_sdl_renderer);
    if (m_sdl_window) SDL_DestroyWindow(m_sdl_window);
    SDL_Quit();
}

bool Window::init_sdl(const std::string& title) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    m_sdl_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_width,
        m_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!m_sdl_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return false;
    }

    m_sdl_renderer = SDL_CreateRenderer(
        m_sdl_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    return m_sdl_renderer != nullptr;
}

bool Window::is_open() const {
    return m_is_running;
}

void Window::poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_is_running = false;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            m_is_running = false;
        }
    }
}

void Window::clear(uint8_t r, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(m_sdl_renderer, r, g, b, 255);
    SDL_RenderClear(m_sdl_renderer);
}

void Window::present() {
    SDL_RenderPresent(m_sdl_renderer);
}

int Window::get_width() const { return m_width; }
int Window::get_height() const { return m_height; }
SDL_Renderer* Window::get_renderer() const { return m_sdl_renderer; }