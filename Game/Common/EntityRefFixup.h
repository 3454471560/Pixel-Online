#pragma once
#include <cstdint>
#include <entt/entity/entity.hpp>
namespace Online::Game
{
    struct EntityRefFixup
    {
        uint32_t bgEntityId = static_cast<uint32_t>(entt::null);
        uint32_t fgEntityId = static_cast<uint32_t>(entt::null);
        uint32_t indicatorEntityId = static_cast<uint32_t>(entt::null);
        uint32_t targetEntityId = static_cast<uint32_t>(entt::null);
        uint32_t spriteEntityId = static_cast<uint32_t>(entt::null);
        uint32_t mainEntityId = static_cast<uint32_t>(entt::null);
        uint32_t overlayEntityId = static_cast<uint32_t>(entt::null);
        bool hasProgressBar = false;
        bool hasFollow = false;
        bool hasAnimator = false;
        bool hasAnimatorController = false;
    };
}