#pragma once
#include <entt/entt.hpp>
#include <glm.hpp>

namespace Online::Physics
{
    struct RayCastHit
    {
        float distance;
        bool hit = false;
        entt::entity entity = entt::null;
        glm::vec2 point;
        glm::vec2 normal;
        float fraction = 0.0f;

    };
}