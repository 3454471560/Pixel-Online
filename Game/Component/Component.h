#pragma once
#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>

#include <entt/entt.hpp>

namespace Online::Game
{
    class GameObject;

    struct Component
    {
        inline GameObject* GetGameObject() const noexcept
        {
            return gameObject;
		}

        inline virtual void OnEnable() {}

        inline virtual void OnDisable() {}

        virtual void Serialize(Online::Serialize::SerializeContext& ctx) const {}
        virtual void Deserialize(const Online::Serialize::DeserializeContext& ctx) {}

        GameObject* gameObject = nullptr;
    };
}