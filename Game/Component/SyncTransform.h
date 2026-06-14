#pragma once
#include "Game/Component/Component.h"
#include <glm.hpp>

namespace Online::Game
{
    struct SyncTransform : public Component
    {
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("x", targetX);
            ctx.Write("y", targetY);
            ctx.Write("rotation", targetRotation);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("x", targetX);
            ctx.Read("y", targetY);
            ctx.Read("rotation", targetRotation);
            needInterpolation = true;
        }

        void OnEnable() override
        {
            IsVisable = true;
		}

        void OnDisable() override
        {
            IsVisable = false;
		}

        bool GetIsActive()
        {
            return IsVisable;
        }
        float targetX = 0.0f;
        float targetY = 0.0f;
        float targetRotation = 0.0f;
        float targetVelX = 0.f;
        float targetVelY = 0.f;
        uint32_t serverFrame = 0;
        bool needInterpolation = false;
        float receiveTime = 0.0f;
		bool IsVisable = true;
    };
}