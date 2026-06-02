#pragma once
#include<Game/Component/Component.h>
#include<Phys/Common/BodyType.h>
#include<Phys/Common/FuncTable.h>

#include<entt/entt.hpp>

namespace Online::Game
{
    struct Rigidbody : public Component 
    {
    public:
		inline void Serialize(Online::Serialize::SerializeContext& ctx) const override
		{
            ctx.Write("type", static_cast<int>(type));
            ctx.Write("fixedRotation", fixedRotation);
            ctx.Write("gravityScale", gravityScale);
            ctx.Write("linearDamping", linearDamping);
            ctx.Write("angularDamping", angularDamping);
            ctx.Write("awake", awake);
		}
		void Deserialize(const Serialize::DeserializeContext& ctx) override
		{
            int t = static_cast<int>(type);
            if (ctx.Read("type", t)) 
            {
                type = static_cast<Online::Physics::BodyType>(t);
            }

            ctx.Read("fixedRotation", fixedRotation);
            ctx.Read("gravityScale", gravityScale);
            ctx.Read("linearDamping", linearDamping);
            ctx.Read("angularDamping", angularDamping);
            ctx.Read("awake", awake);
		}       
    public:
        inline static glm::vec2 GetVelocity(const entt::entity& ID)
        {
            return Physics::GetLinearVelocity(ID);
        }
        inline static void SetVelocity(const entt::entity& ID, const glm::vec2& vel)
        {
            Physics::SetLinearVelocity(ID, vel);
        }

        inline float GetAngularVelocity(const entt::entity& ID) const
        {
            return Physics::GetAngularVelocity(ID);
        }
        inline void SetAngularVelocity(const entt::entity& ID, float omega)
        {
            Physics::SetAngularVelocity(ID, omega);
        }

        inline void AddForce(const entt::entity& ID, const glm::vec2& force)
        {
            Physics::ApplyForce(ID, force);
        }
        inline void AddForceAtPoint(const entt::entity& ID, const glm::vec2& force, const glm::vec2& point)
        {
            Physics::ApplyForceAtPoint(ID, force, point);
        }
        inline void AddImpulse(const entt::entity& ID, const glm::vec2& impulse)
        {
            Physics::ApplyLinearImpulse(ID, impulse);
        }
        inline void AddImpulseAtPoint(const entt::entity& ID, const glm::vec2& impulse, const glm::vec2& point)
        {
            Physics::ApplyLinearImpulseAtPoint(ID, impulse, point);
        }

        inline void WakeUp(const entt::entity& ID)
        {
            Physics::SetAwake(ID, true);
        }
        inline void Sleep(const entt::entity& ID)
        {
            Physics::SetAwake(ID, false);
        }
    public:
        inline Online::Physics::BodyType GetBodyType()
        {
            return type;
        }
        inline void SetBodyType(Online::Physics::BodyType t)
        {
            type = t;
        }
        inline bool IsFixRotation()
        {
            return fixedRotation;
        }
        inline void SetFixedRotation(bool v)
        {
            fixedRotation = v;
        }
        inline float getGravityScale()
        {
            return gravityScale;
        }
        inline void SetGravityScale(float v)
        {
            gravityScale = v;
        }
        inline float GetLinearDamping()
        {
            return linearDamping;
        }
        inline void SetLinearDamping(float v)
        {
            linearDamping = v;
        }
        inline float GetAngularDamping()
        {
            return angularDamping;
        }
        inline void SetAngularDamping(float v)
        {
            angularDamping = v;
        }
        inline bool IsAwake()
        {
            return awake;
        }
        inline void SetAwake(bool v)
        {
            awake = v;
        }
    public:
    private:
        Online::Physics::BodyType type = Online::Physics::BodyType::Dynamic;
        bool fixedRotation = false;
        float gravityScale = 1.0f;
        float linearDamping = 0.0f;
        float angularDamping = 0.0f;
        bool awake = true;
    };
}
