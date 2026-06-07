#pragma once

#include<SDL.h>
#include<glm.hpp>

namespace Online::Core
{
    struct CameraSnapshot
    {
        glm::vec2  Position = { 0, 0 };
        float      Zoom = 1.0f;
        float      Rotation = 0.0f;
        bool       IsWorld = true;

        CameraSnapshot(glm::vec2 position)
            : Position(position)
        {
        }

        CameraSnapshot() = default;

        glm::vec2 WorldToScreen(const glm::vec2& worldPos) const
        {
            float dx = worldPos.x - Position.x;
            float dy = worldPos.y - Position.y;

            float rad = glm::radians(Rotation);
            float cosA = std::cos(rad);
            float sinA = std::sin(rad);

            float rx = dx * cosA - dy * sinA;
            float ry = dx * sinA + dy * cosA;

            float sx = rx * Zoom;
            float sy = ry * Zoom;

            return { sx, sy };
        }

        bool IsInViewAABB(const SDL_FRect& DstRect, const SDL_FPoint& Pivot, const glm::ivec2& viewportSize) const
        {
            if (Rotation == 0.0f)
            {
                glm::vec2 screenPos = WorldToScreen({ DstRect.x, DstRect.y });
                float w = DstRect.w * Zoom;
                float h = DstRect.h * Zoom;

                if (screenPos.x + w < 0.0f) return false;
                if (screenPos.y + h < 0.0f) return false;
                if (screenPos.x > static_cast<float>(viewportSize.x)) return false;
                if (screenPos.y > static_cast<float>(viewportSize.y)) return false;

                return true;
            }

            float pivotWorldX = DstRect.x + Pivot.x;
            float pivotWorldY = DstRect.y + Pivot.y;

            float left = -Pivot.x;
            float right = DstRect.w - Pivot.x;
            float top = Pivot.y;
            float bottom = DstRect.h - Pivot.y;

            float rad = glm::radians(Rotation);
            float cosA = std::cos(rad);
            float sinA = std::sin(rad);

            glm::vec2 worldVerts[4] = {
                { pivotWorldX + left * cosA - top * sinA,    pivotWorldY + left * sinA + top * cosA },
                { pivotWorldX + right * cosA - top * sinA,   pivotWorldY + right * sinA + top * cosA },
                { pivotWorldX + left * cosA - bottom * sinA, pivotWorldY + left * sinA + bottom * cosA },
                { pivotWorldX + right * cosA - bottom * sinA,pivotWorldY + right * sinA + bottom * cosA } 
            };

            float minX = 999999.0f, maxX = -999999.0f;
            float minY = 999999.0f, maxY = -999999.0f;

            for (int i = 0; i < 4; ++i)
            {
                glm::vec2 screenV = WorldToScreen(worldVerts[i]);
                if (screenV.x < minX) minX = screenV.x;
                if (screenV.x > maxX) maxX = screenV.x;
                if (screenV.y < minY) minY = screenV.y;
                if (screenV.y > maxY) maxY = screenV.y;
            }

            if (maxX < 0.0f) return false;
            if (maxY < 0.0f) return false;
            if (minX > static_cast<float>(viewportSize.x)) return false;
            if (minY > static_cast<float>(viewportSize.y)) return false;

            return true;
        }
    };

}
