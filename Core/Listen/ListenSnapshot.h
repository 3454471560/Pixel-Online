#pragma once
#include <glm.hpp>

namespace Online::Core
{
    struct ListenSnapshot
    {
        glm::vec2 Position = { 0.0f, 0.0f };
        float Range = 100.0f;
        float MasterVolume = 1.0f;

        ListenSnapshot() = default;

        explicit ListenSnapshot(glm::vec2 position)
            : Position(position)
        {
        }

        ~ListenSnapshot() = default;

        float WorldToDistance(const glm::vec2& worldPos) const
        {
            return glm::distance(Position, worldPos);
        }

        bool IsInListenRange(const glm::vec2& worldPos) const
        {
            return WorldToDistance(worldPos) <= Range;
        }
    };
}