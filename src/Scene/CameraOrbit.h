#pragma once

#include "Math/Vec3.h"

// Keeps the camera on a sphere around a target: azimuth and elevation set where on the
// sphere, radius sets how far away. Rotates slowly on its own unless turned off.
class CameraOrbit {
    public:
        CameraOrbit(double radius, double elevation_degrees, double angular_speed,
                    double min_radius, double max_radius, const Vec3& target = Vec3());

        // Advances the automatic rotation by real elapsed time
        void advance(double dt_seconds);

        // Turns the camera around the target by the given angles, in radians
        void rotate(double d_azimuth, double d_elevation);

        // Moves closer (factor < 1) or further away (factor > 1)
        void zoom(double factor);

        void toggle_auto_rotate() { m_auto_rotate = !m_auto_rotate; }

        Vec3 position() const;
        Vec3 target() const { return m_target; }

    private:
        double m_radius;
        double m_elevation;
        double m_azimuth;
        double m_angular_speed; // Radians per second
        double m_min_radius;
        double m_max_radius;
        bool m_auto_rotate;
        Vec3 m_target;
};
