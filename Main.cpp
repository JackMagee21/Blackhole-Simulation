#include "Window.h"
#include "PixelBuffer.h"
#include "Colour.h"
#include <cmath>

int main() {
    const int screen_width = 800;
    const int screen_height = 600;

    Window window("Black Hole Simulation 0.02", screen_width, screen_height);
    PixelBuffer buffer(window.get_renderer(), screen_width, screen_height);
    

    double t = 0.0;

    while (window.is_open()) {
        window.poll_events();

        t += 0.02;
        int center_x = screen_width / 2;
        int center_y = screen_height / 2;

        for (int y = 0; y < screen_height; ++y) {
            for (int x = 0; x < screen_width; ++x) {
                double dx = x - center_x;
                double dy = y - center_y;
                double dist = std::sqrt(dx * dx + dy * dy);

                // Intensity falls off with distance, oscillating in time
                double wave = (std::sin(dist * 0.04 - t) + 1.0) * 0.5;
                
                // Colour math using operator* and operator+
                Colour core_colour(0.0, 0.0, 0.0); // Warm accretion disk orange
                Colour edge_colour(1.0, 1.0, 1.0); // Blue gravitational shift

                Colour blended = (core_colour * wave) + (edge_colour * (1.0 - wave));

                buffer.set_pixel(x, y, blended);
            }
        }

        window.clear();
        buffer.render();
        window.present();
    }

    return 0;
}