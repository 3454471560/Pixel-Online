#pragma once
#include <Game/Component/Component.h>
#include <Game/Component/Transform.h>
#include <entt/entt.hpp>
#include <glm.hpp>

namespace Online::Game
{
    enum class FollowMode : uint8_t
    {
        Direct,
        Linear,
        Smooth
    };

    struct Follow : public Component
    {
    public:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("mode", static_cast<uint8_t>(mode));
            ctx.Write("enableFollow", enableFollow);
            ctx.Write("offset", offset);
            ctx.Write("linearSpeed", linearSpeed);
            ctx.Write("smoothTime", smoothTime);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            uint8_t m = 0; ctx.Read("mode", m); mode = static_cast<FollowMode>(m);
            ctx.Read("enableFollow", enableFollow);
            ctx.Read("offset", offset);
            ctx.Read("linearSpeed", linearSpeed);
            ctx.Read("smoothTime", smoothTime);
        }
    public:
        inline void SetFollowMode(FollowMode mode)
        {
            this->mode = mode;
        }
        inline void SetEnableFollow(bool flag)
        {
            enableFollow = flag;
        }
        inline void SetTarget(Transform* transform)
        {
            target = transform; 
        }
        inline void SetOffest(glm::vec2 offset)
        {
            this->offset = offset;
        }
        inline void SetLinearSpeed(float speed)
        {
            linearSpeed = speed;
        }
        inline void SetSmoothTime(float time)
        {
            smoothTime = time;
        }
        inline void GetVelocity(glm::vec2 velocity)
        {
            currentVelocity = velocity;
        }
    public:
        inline FollowMode GetFollowMode()
        {
            return mode;
        }
        inline bool GetEnableFollow()
        {
            return enableFollow;
        }
        inline Transform* GetTarget() const 
        {
            return target;
        }
        inline glm::vec2 GetOffest()
        {
            return offset;
        }
        inline float GetLinearSpeed()
        {
            return linearSpeed;
        }
        inline float GetSmoothTime()
        {
            return smoothTime;
        }
        inline glm::vec2 GetVelocity()
        {
            return currentVelocity;
        }
    private:
        Transform* target = nullptr;
        bool enableFollow = true;
        FollowMode mode = FollowMode::Linear;
        float linearSpeed = 5.0f;
        float smoothTime = 0.15f;
        glm::vec2 offset = { 0, 0 };
        glm::vec2 currentVelocity = { 0, 0 };
    };
}