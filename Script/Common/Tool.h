#pragma once
#include <glm.hpp>
#include <SDL.h>

namespace Online::Script
{
    static bool PointInRect(const glm::vec2& point, const SDL_FRect& rect)
    {
        return point.x >= rect.x && point.x <= rect.x + rect.w &&
            point.y >= rect.y && point.y <= rect.y + rect.h;
    }
}

