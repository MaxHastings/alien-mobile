#pragma once
#include "alienmobile/Math.h"
#include <algorithm>
namespace alienmobile {
// Zoom is clip-space units per world unit along the shorter viewport axis.
// UIKit input uses points; the Metal shader uses pixels. Only aspect ratio
// enters either mapping, so Retina scale cannot change the camera.
inline Vec2 worldFromScreen(Vec2 point, Vec2 viewport, Vec2 center, float zoom)
{
    float minimum = std::max(1.0f, std::min(viewport.x, viewport.y));
    float units = 2.0f / (minimum * zoom);
    return {center.x + (point.x - viewport.x * 0.5f) * units,
            center.y - (point.y - viewport.y * 0.5f) * units};
}
inline Vec2 screenFromWorld(Vec2 point, Vec2 viewport, Vec2 center, float zoom)
{
    float scale = std::max(1.0f, std::min(viewport.x, viewport.y)) * zoom * 0.5f;
    return {viewport.x * 0.5f + (point.x-center.x)*scale,
            viewport.y * 0.5f - (point.y-center.y)*scale};
}
}
