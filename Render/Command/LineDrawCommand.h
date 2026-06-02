#pragma once
#include <Core/Color/Color.h>
#include <glm.hpp>
#include <SDL.h>

namespace Online::Render
{
    struct LineDrawCommand
    {
        glm::vec2 StartPoint = { 0.0f, 0.0f };
        glm::vec2 EndPoint = { 0.0f, 0.0f };
        Online::Core::Color Color = Online::Core::Color::White;
        float Thickness = 1.0f;

        LineDrawCommand() = default;

        LineDrawCommand(const glm::vec2& start, const glm::vec2& end, const Online::Core::Color& color, float thickness = 1.0f)
            : StartPoint(start)
            , EndPoint(end)
            , Color(color)
            , Thickness(thickness)
        {
        }
    };
}