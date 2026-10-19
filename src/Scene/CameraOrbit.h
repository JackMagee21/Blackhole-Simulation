#pragma once

#include "Physics/Vec3/Vec3.h"

// Moves the camera in a horizontal circle around a target at a fixed height
class CameraOrbit {
    public:
        CameraOrbit(double radius, double height, double angular_speed, const Vec3& target = Vec3());

        // Advances by real elapsed time so the speed doesn't depend on the frame rate
        void advance(double dt_seconds);

        Vec3 position() const;
        Vec3 target() const { return m_target; }

    private:
        double m_radius;
        double m_height;
        double m_angular_speed; // Radians per second
        double m_angle;
        Vec3 m_target;
};
