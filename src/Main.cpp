#include "Platform/Window.h"
#include "Render/Image.h"
#include "Render/PixelBuffer.h"
#include "Render/Renderer.h"
#include "Scene/Camera.h"
#include "Scene/CameraOrbit.h"
#include "Scene/SceneConfig.h"
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

    const char* window_title = "Relativistic Black Hole - Schwarzschild Geodesics";

    CameraOrbit make_orbit(const SceneConfig& config) {
        const double M = config.black_hole_mass;
        return CameraOrbit(config.orbit_radius_in_M * M, config.camera_elevation_degrees, config.orbit_speed,
                           config.min_orbit_radius_in_M * M, config.max_orbit_radius_in_M * M);
    }

    // Renders one frame `seconds` into the run without opening a window and saves it as a PPM
    int save_screenshot(const SceneConfig& config, const std::string& path, double seconds) {
        CameraOrbit orbit = make_orbit(config);
        orbit.advance(seconds);
        Camera camera(orbit.position(), orbit.target(), config.fov_degrees, config.aspect_ratio());
        Renderer renderer(config);

        // There is no frame rate to keep up, so take the time to smooth edges properly
        renderer.settings().supersampling = 3;

        Image image(config.screen_width, config.screen_height);
        renderer.render_frame(camera, seconds * config.time_scale_in_M * config.black_hole_mass, image);

        if (!image.save_ppm(path)) {
            std::fprintf(stderr, "Could not write %s\n", path.c_str());
            return 1;
        }
        std::printf("Saved %s\n", path.c_str());
        return 0;
    }

    void print_controls() {
        std::printf(
            "Controls:\n"
            "  Left drag   orbit the camera\n"
            "  Scroll      zoom in / out\n"
            "  R           toggle automatic camera orbit\n"
            "  Space       pause / resume time\n"
            "  D           toggle accretion disk\n"
            "  G           toggle checkerboard sky (shows lensing distortion)\n"
            "  B           toggle bloom\n"
            "  A           toggle 2x2 anti-aliasing (4x slower)\n"
            "  - / =       exposure down / up\n"
            "  Esc         quit\n");
    }

    void handle_input(const InputState& input, CameraOrbit& orbit, RenderSettings& settings, bool& paused) {
        const double drag_speed = 0.005; // Radians per pixel
        if (input.mouse_dx != 0 || input.mouse_dy != 0) {
            orbit.rotate(-input.mouse_dx * drag_speed, input.mouse_dy * drag_speed);
        }
        if (input.wheel != 0.0) {
            orbit.zoom(std::pow(0.9, input.wheel));
        }

        for (char key : input.keys_pressed) {
            switch (key) {
                case ' ': paused = !paused; break;
                case 'r': orbit.toggle_auto_rotate(); break;
                case 'd': settings.show_disk = !settings.show_disk; break;
                case 'g': settings.show_grid = !settings.show_grid; break;
                case 'b': settings.bloom = !settings.bloom; break;
                case 'a': settings.supersampling = settings.supersampling == 1 ? 2 : 1; break;
                case '=': settings.exposure *= 1.25; break;
                case '-': settings.exposure /= 1.25; break;
                default: break;
            }
        }
    }

} // namespace

int main(int argc, char* argv[]) {
    SceneConfig config;

    // Usage: sim --screenshot out.ppm [seconds]
    if (argc >= 3 && std::string(argv[1]) == "--screenshot") {
        double seconds = argc >= 4 ? std::atof(argv[3]) : 0.0;
        return save_screenshot(config, argv[2], seconds);
    }

    Window window(window_title, config.screen_width, config.screen_height);
    PixelBuffer buffer(window.get_renderer(), config.screen_width, config.screen_height);

    CameraOrbit orbit = make_orbit(config);
    Camera camera(orbit.position(), orbit.target(), config.fov_degrees, config.aspect_ratio());
    Renderer renderer(config);
    print_controls();

    using Clock = std::chrono::steady_clock;
    Clock::time_point last_frame = Clock::now();
    Clock::time_point last_title = last_frame;
    int frames = 0;
    double sim_time = 0.0;
    bool paused = false;

    while (window.is_open()) {
        window.poll_events();
        handle_input(window.input(), orbit, renderer.settings(), paused);

        Clock::time_point now = Clock::now();
        double dt = std::chrono::duration<double>(now - last_frame).count();
        last_frame = now;

        if (!paused) {
            sim_time += dt * config.time_scale_in_M * config.black_hole_mass;
        }

        // Orbit the camera smoothly around the black hole
        orbit.advance(dt);
        camera.look_at(orbit.position(), orbit.target());

        renderer.render_frame(camera, sim_time, buffer);

        window.clear();
        buffer.render();
        window.present();

        // Show the frame rate in the title bar once a second
        ++frames;
        double since_title = std::chrono::duration<double>(now - last_title).count();
        if (since_title >= 1.0) {
            char title[160];
            std::snprintf(title, sizeof(title), "%s - %.0f FPS", window_title, frames / since_title);
            window.set_title(title);
            frames = 0;
            last_title = now;
        }
    }

    return 0;
}
