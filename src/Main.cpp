#include "Platform/Window.h"
#include "Render/PixelBuffer.h"
#include "Scene/Camera.h"
#include "Render/Renderer.h"
#include "Scene/CameraOrbit.h"
#include "Scene/SceneConfig.h"
#include <chrono>

int main() {
    SceneConfig config;
    const double M = config.black_hole_mass;

    Window window("Relativistic Black Hole - Schwarzschild Geodesics", config.screen_width, config.screen_height);
    PixelBuffer buffer(window.get_renderer(), config.screen_width, config.screen_height);

    CameraOrbit orbit(config.orbit_radius_in_M * M, config.camera_height_in_M * M, config.orbit_speed);
    Camera camera(orbit.position(), orbit.target(), config.fov_degrees, config.aspect_ratio());
    Renderer renderer(M);

    using Clock = std::chrono::steady_clock;
    Clock::time_point last_frame = Clock::now();

    while (window.is_open()) {
        window.poll_events();

        // Orbit the camera smoothly around the black hole
        Clock::time_point now = Clock::now();
        orbit.advance(std::chrono::duration<double>(now - last_frame).count());
        last_frame = now;
        camera.look_at(orbit.position(), orbit.target());

        renderer.render_frame(camera, buffer);

        window.clear();
        buffer.render();
        window.present();
    }

    return 0;
}
