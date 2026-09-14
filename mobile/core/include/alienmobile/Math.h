#pragma once

#include <cmath>

namespace alienmobile {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2& operator+=(Vec2 const& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-=(Vec2 const& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }
};

inline Vec2 operator+(Vec2 left, Vec2 const& right)
{
    return left += right;
}

inline Vec2 operator-(Vec2 left, Vec2 const& right)
{
    return left -= right;
}

inline Vec2 operator-(Vec2 value)
{
    return {-value.x, -value.y};
}

inline Vec2 operator*(Vec2 value, float scalar)
{
    return value *= scalar;
}

inline Vec2 operator*(float scalar, Vec2 value)
{
    return value *= scalar;
}

inline Vec2 operator/(Vec2 value, float scalar)
{
    if (scalar == 0.0f) {
        return {};
    }
    value *= 1.0f / scalar;
    return value;
}

inline float dot(Vec2 const& left, Vec2 const& right)
{
    return left.x * right.x + left.y * right.y;
}

inline float lengthSquared(Vec2 const& value)
{
    return dot(value, value);
}

inline float length(Vec2 const& value)
{
    return std::sqrt(lengthSquared(value));
}

inline Vec2 normalizedOr(Vec2 const& value, Vec2 fallback = {1.0f, 0.0f})
{
    auto const valueLength = length(value);
    if (!std::isfinite(valueLength) || valueLength < 0.000001f) {
        return fallback;
    }
    return value / valueLength;
}

inline bool isFinite(Vec2 const& value)
{
    return std::isfinite(value.x) && std::isfinite(value.y);
}

inline float clamp(float value, float minimum, float maximum)
{
    return value < minimum ? minimum : (value > maximum ? maximum : value);
}

inline bool nearlyEqual(float left, float right, float epsilon = 0.0001f)
{
    return std::fabs(left - right) <= epsilon;
}

inline bool operator==(Vec2 const& left, Vec2 const& right)
{
    return left.x == right.x && left.y == right.y;
}

inline bool operator!=(Vec2 const& left, Vec2 const& right)
{
    return !(left == right);
}

} // namespace alienmobile
