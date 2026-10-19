#pragma once

#include "Math/Vec3.h"
#include "Math/Ray.h"

class Camera {
    public:
        Camera(const Vec3& pos, const Vec3& look_at, double fov_degrees, double aspect_ratio);

        // Setters
        void look_at(const Vec3& pos, const Vec3& target);
        void set_aspect_ratio(double aspect_ratio);
        void set_fov(double fov_degrees);

        // Getters
        Vec3 get_position() const { return m_position; }
        Vec3 get_forward() const  { return m_forward; }
        Vec3 get_right() const    { return m_right; }
        Vec3 get_up() const       { return m_up; }
        
        Ray generate_ray(double x, double y, int width, int height) const;
    private:
        Vec3 m_position;
        Vec3 m_target;
        double m_fov_degrees;
        double m_aspect_ratio;

        Vec3 m_forward;
        Vec3 m_right;
        Vec3 m_up;

        double m_viewport_scale;

        void update_basis();
};
