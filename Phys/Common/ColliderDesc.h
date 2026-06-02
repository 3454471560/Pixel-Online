#pragma once
#include <Phys/Common/ColliderShape.h>

#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <glm.hpp>

namespace Online::Physics
{
    struct ColliderDesc
    {
        entt::entity entity;
        ColliderShape shapeType = ColliderShape::Box;
        glm::vec2 offset = { 0.0f, 0.0f };
        float radius = 0.5f;
        glm::vec2 halfSize = { 0.5f, 0.5f };
        float angle = 0.0f;

        float density = 1.0f;
        float friction = 0.3f;
        float restitution = 0.0f;
        bool isSensor = false;

        uint64_t categoryBits = 0x0001;
        uint64_t maskBits = 0xFFFF;
        int groupIndex = 0;
    };
}