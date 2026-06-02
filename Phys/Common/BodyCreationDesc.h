#pragma once
#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <glm.hpp>

namespace Online::Physics
{
    struct BodyCreationDesc
    {
        entt::entity entity;
        b2BodyType type = b2_dynamicBody;
        glm::vec2 position = { 0.0f, 0.0f };
        float angle = 0.0f;
        bool fixedRotation = false;
        float gravityScale = 1.0f;
        float linearDamping = 0.0f;
        float angularDamping = 0.0f;
        bool awake = true;
    };
}