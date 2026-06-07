#pragma once
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Game/Entity/GameObject.h>
#include <Game/Component/Transform.h>
#include <Game/Component/Follow.h>
#include <glm.hpp>

namespace Online::Script
{
    struct Follow
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::FollowOverTime;

        class FollowData
        {
            friend Follow;
        public:
            inline void SetTarget(Game::Transform* targetTransform) { target = targetTransform; }
            inline void SetOffset(const glm::vec2& offset) { this->offset = offset; }
            inline void SetMode(Game::FollowMode mode) { this->mode = mode; }
            inline void SetLinearSpeed(float speed) { linearSpeed = speed; }
            inline void SetSmoothTime(float time) { smoothTime = time; }

        private:
            Game::Transform* target = nullptr;
            glm::vec2 offset{ 0.0f, 0.0f };
            Game::FollowMode mode = Game::FollowMode::Direct;
            float linearSpeed = 5.0f;
            float smoothTime = 0.15f;
            glm::vec2 currentVelocity{ 0.0f, 0.0f };
        };

        static void FollowData_Construct(void* p)
        {
            new (p) FollowData();
        }
        static void FollowData_Destruct(void* p)
        {
            static_cast<FollowData*>(p)->~FollowData();
        }

        static void FollowData_OnEnable(Game::GameObject* go) {}

        static void Follow_LateUpdate(Game::GameObject* go, float dt)
        {
            auto* data = go->GetScriptData<FollowData>(ID);
            if (!data) return;

            auto* selfTrans = go->GetTransform();
            if (!selfTrans || !data->target) return;

            glm::vec2 targetPos = data->target->GetWorldPosition() + data->offset;
            glm::vec2 currentPos = selfTrans->GetWorldPosition();
            glm::vec2 newPos;

            switch (data->mode)
            {
            case Game::FollowMode::Direct:
                newPos = targetPos;
                break;
            case Game::FollowMode::Linear:
            {
                float t = data->linearSpeed * dt;
                if (t > 1.0f) t = 1.0f;
                newPos = glm::mix(currentPos, targetPos, t);
                break;
            }
            case Game::FollowMode::Smooth:
            {
                float smoothFactor = 1.0f - glm::exp(-dt / data->smoothTime);
                newPos = glm::mix(currentPos, targetPos, smoothFactor);
                break;
            }
            }

            selfTrans->SetWorldPosition(newPos);
        }

        static ScriptFunctionInfo Information()
        {
            return
            {
                ID,
                sizeof(FollowData),
                FollowData_Construct,
                FollowData_Destruct,
                FollowData_OnEnable,
                nullptr,                // OnDisable
                nullptr,                // Update (Пе)
                Follow_LateUpdate       // LateUpdate
            };
        }
    };
}