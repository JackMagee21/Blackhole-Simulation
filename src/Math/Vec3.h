#pragma once

#include <cmath>

struct Vec3 {
    double x;
    double y;
    double z;

    constexpr Vec3(double x = 0.0, double y = 0.0, double z = 0.0)
        : x(x), y(y), z(z) {}

    // Basic arithmetic
    Vec3 operator+(const Vec3& b) const { return {x + b.x, y + b.y, z + b.z}; }
    Vec3 operator-(const Vec3& b) const { return {x - b.x, y - b.y, z - b.z}; }
    Vec3 operator*(double s) const      { return {x * s, y * s, z * s}; }
    Vec3 operator/(double s) const      { return {x / s, y / s, z / s}; }
    Vec3 operator-() const              { return {-x, -y, -z}; }

    Vec3& operator+=(const Vec3& b) { x += b.x; y += b.y; z += b.z; return *this; }
    Vec3& operator-=(const Vec3& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
    Vec3& operator*=(double s)      { x *= s; y *= s; z *= s; return *this; }

    // Dot product: measures alignment
    double dot(const Vec3& b) const {
        return x * b.x + y * b.y + z * b.z;
    }

    // Cross product: finds perpendicular vector (vital for angular momentum L = r x v)
    Vec3 cross(const Vec3& b) const {
        return {
            y * b.z - z * b.y,
            z * b.x - x * b.z,
            x * b.y - y * b.x
        };
    }

    // Vector length
    double norm() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    double norm_squared() const {
        return x * x + y * y + z * z;
    }

    // Returns direction with length 1.0
    Vec3 normalized() const {
        double len = norm();
        return (len > 1e-9) ? (*this / len) : Vec3(0, 0, 0);
    }
};

// Lets scalars go on either side, e.g. 2.0 * v
inline Vec3 operator*(double s, const Vec3& v) { return v * s; }