#pragma once
#include <Context/Context.h>
#include <Core/StateFlags/StateFlags.h>
#include <Phys/Common/RayCastHit.h>
#include <Phys/Common/PhysicsLayer.h>

#include <entt/entt.hpp>
#include <glm.hpp>

#include <stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Physics::PhysicsSimulator>
    {
        friend class Online::Runtime::Client;
        friend class Online::Runtime::Server;
    private:
        FuncTable() = default;
        ~FuncTable() = default;

    public:
        FuncTable(const FuncTable&) = delete;
        FuncTable& operator=(const FuncTable&) = delete;

    public:
        bool Check() const
        {
            if (!OnRemoveBody) { throw std::runtime_error("FuncTable miss [Physics::RemoveBody] Function!"); }
            if (!OnSetGravity) { throw std::runtime_error("FuncTable miss [Physics::SetGravity] Function!"); }
            if (!OnGetLinearVelocity) { throw std::runtime_error("FuncTable miss [Physics::GetLinearVelocity] Function!"); }
            if (!OnSetLinearVelocity) { throw std::runtime_error("FuncTable miss [Physics::SetLinearVelocity] Function!"); }
            if (!OnGetAngularVelocity) { throw std::runtime_error("FuncTable miss [Physics::GetAngularVelocity] Function!"); }
            if (!OnSetAngularVelocity) { throw std::runtime_error("FuncTable miss [Physics::SetAngularVelocity] Function!"); }
            if (!OnApplyForce) { throw std::runtime_error("FuncTable miss [Physics::ApplyForce] Function!"); }
            if (!OnApplyForceAtPoint) { throw std::runtime_error("FuncTable miss [Physics::ApplyForceAtPoint] Function!"); }
            if (!OnApplyLinearImpulse) { throw std::runtime_error("FuncTable miss [Physics::ApplyLinearImpulse] Function!"); }
            if (!OnApplyLinearImpulseAtPoint) { throw std::runtime_error("FuncTable miss [Physics::ApplyLinearImpulseAtPoint] Function!"); }
            if (!OnIsAwake) { throw std::runtime_error("FuncTable miss [Physics::IsAwake] Function!"); }
            if (!OnSetAwake) { throw std::runtime_error("FuncTable miss [Physics::SetAwake] Function!"); }
            if (!OnAddDebugRay) { throw std::runtime_error("FuncTable miss [Physics::AddDebugRay] Function!"); }
            if (!OnRayCast) { throw std::runtime_error("FuncTable miss [Physics::OnRayCast] Function!"); }
            if (!OnRayCastLayer) { throw std::runtime_error("FuncTable miss [Physics::OnRayCastLayer] Function!"); }
            if (!OnSetBodyTransform) { throw std::runtime_error("FuncTable miss [Physics::SetBodyTransform] Function!"); }
            return true;
        }
        void UnRegister() noexcept
        {
            OnRemoveBody = nullptr;
            OnSetGravity = nullptr;
            OnGetLinearVelocity = nullptr;
            OnSetLinearVelocity = nullptr;
            OnGetAngularVelocity = nullptr;
            OnSetAngularVelocity = nullptr;
            OnApplyForce = nullptr;
            OnApplyForceAtPoint = nullptr;
            OnApplyLinearImpulse = nullptr;
            OnApplyLinearImpulseAtPoint = nullptr;
            OnIsAwake = nullptr;
            OnSetAwake = nullptr;
            OnAddDebugRay = nullptr;
            OnRayCast = nullptr;
            OnRayCastLayer = nullptr;
            OnSetBodyTransform = nullptr;
        }

    public:
        void InvokeOnRemoveBody(entt::entity entity) const 
        { 
            OnRemoveBody(entity); 
        }
        void InvokeOnSetGravity(glm::vec2 gravity) const 
        { 
            OnSetGravity(gravity); 
        }
        glm::vec2 InvokeOnGetLinearVelocity(entt::entity entity) const 
        { 
            return OnGetLinearVelocity(entity); 
        }
        void InvokeOnSetLinearVelocity(entt::entity entity, const glm::vec2& velocity) const 
        { 
            OnSetLinearVelocity(entity, velocity); 
        }
        float InvokeOnGetAngularVelocity(entt::entity entity) const 
        { 
            return OnGetAngularVelocity(entity); 
        }
        void InvokeOnSetAngularVelocity(entt::entity entity, float omega) const 
        {
            OnSetAngularVelocity(entity, omega); 
        }
        void InvokeOnApplyForce(entt::entity entity, const glm::vec2& force) const 
        { 
            OnApplyForce(entity, force); 
        }
        void InvokeOnApplyForceAtPoint(entt::entity entity, const glm::vec2& force, const glm::vec2& worldPoint) const 
        { 
            OnApplyForceAtPoint(entity, force, worldPoint); 
        }
        void InvokeOnApplyLinearImpulse(entt::entity entity, const glm::vec2& impulse) const 
        { 
            OnApplyLinearImpulse(entity, impulse); 
        }
        void InvokeOnApplyLinearImpulseAtPoint(entt::entity entity, const glm::vec2& impulse, const glm::vec2& worldPoint) const 
        { 
            OnApplyLinearImpulseAtPoint(entity, impulse, worldPoint); 
        }
        bool InvokeOnIsAwake(entt::entity entity) const 
        { 
            return OnIsAwake(entity); 
        }
        void InvokeOnSetAwake(entt::entity entity, bool awake) const 
        { 
            OnSetAwake(entity, awake); 
        }
        void InvokeOnAddDebugRay(glm::vec2 origin, glm::vec2 direction, float length, glm::vec4 color) const
        {
            OnAddDebugRay(origin, direction, length, color);
        }
        bool InvokeOnRayCast(glm::vec2 origin, glm::vec2 angleRad, float maxDistance, Physics::RayCastHit& outHit) const
        {
            return OnRayCast(origin, angleRad, maxDistance, outHit);
        }
        bool InvokeOnRayCastLayer(glm::vec2 origin, glm::vec2 angleRad, float maxDistance, Physics::RayCastHit& outHit, uint16_t layerMask, bool includeTriggers) const
        {
            return OnRayCastLayer(origin, angleRad, maxDistance, outHit, layerMask, includeTriggers);
        }
        void InvokeOnSetBodyTransform(entt::entity entity, const glm::vec2& position, float angle) const
        {
            OnSetBodyTransform(entity, position, angle);
        }
    public:
        void(*OnRemoveBody)(entt::entity) = nullptr;
        void(*OnSetGravity)(glm::vec2) = nullptr;
        glm::vec2(*OnGetLinearVelocity)(entt::entity) = nullptr;
        void(*OnSetLinearVelocity)(entt::entity, const glm::vec2&) = nullptr;
        float(*OnGetAngularVelocity)(entt::entity) = nullptr;
        void(*OnSetAngularVelocity)(entt::entity, float) = nullptr;
        void(*OnApplyForce)(entt::entity, const glm::vec2&) = nullptr;
        void(*OnApplyForceAtPoint)(entt::entity, const glm::vec2&, const glm::vec2&) = nullptr;
        void(*OnApplyLinearImpulse)(entt::entity, const glm::vec2&) = nullptr;
        void(*OnApplyLinearImpulseAtPoint)(entt::entity, const glm::vec2&, const glm::vec2&) = nullptr;
        bool(*OnIsAwake)(entt::entity) = nullptr;
        void(*OnSetAwake)(entt::entity, bool) = nullptr;
        void(*OnAddDebugRay)(glm::vec2, glm::vec2, float, glm::vec4) = nullptr;
        bool(*OnRayCast)(glm::vec2, glm::vec2, float, Physics::RayCastHit&) = nullptr;
        bool(*OnRayCastLayer)(glm::vec2, glm::vec2, float, Physics::RayCastHit&, uint16_t, bool) = nullptr;
        void(*OnSetBodyTransform)(entt::entity, const glm::vec2&, float) = nullptr;
    };
}

namespace Online::Physics
{
    inline void RemoveBody(entt::entity entity)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnRemoveBody(entity);
    }

    inline void SetGravity(glm::vec2 gravity)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnSetGravity(gravity);
    }

    inline glm::vec2 GetLinearVelocity(entt::entity entity)
    {
        return Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnGetLinearVelocity(entity);
    }

    inline void SetLinearVelocity(entt::entity entity, const glm::vec2& velocity)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnSetLinearVelocity(entity, velocity);
    }

    inline float GetAngularVelocity(entt::entity entity)
    {
        return Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnGetAngularVelocity(entity);
    }

    inline void SetAngularVelocity(entt::entity entity, float omega)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnSetAngularVelocity(entity, omega);
    }

    inline void ApplyForce(entt::entity entity, const glm::vec2& force)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnApplyForce(entity, force);
    }

    inline void ApplyForceAtPoint(entt::entity entity, const glm::vec2& force, const glm::vec2& worldPoint)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnApplyForceAtPoint(entity, force, worldPoint);
    }

    inline void ApplyLinearImpulse(entt::entity entity, const glm::vec2& impulse)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnApplyLinearImpulse(entity, impulse);
    }

    inline void ApplyLinearImpulseAtPoint(entt::entity entity, const glm::vec2& impulse, const glm::vec2& worldPoint)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnApplyLinearImpulseAtPoint(entity, impulse, worldPoint);
    }

    inline bool IsAwake(entt::entity entity)
    {
        return Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnIsAwake(entity);
    }

    inline void SetAwake(entt::entity entity, bool awake)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnSetAwake(entity, awake);
    }

    inline void SetBodyTransform(entt::entity entity, const glm::vec2& position, float angle)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnSetBodyTransform(entity, position, angle);
    }

    inline void AddDebugRay(glm::vec2 origin, glm::vec2 direction, float length, glm::vec4 color)
    {
        Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnAddDebugRay(origin, direction, length, color);
    }

    inline bool RayCast(glm::vec2 origin, glm::vec2 angleRad, float maxDistance, RayCastHit& outHit)
    {
        return Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnRayCast(origin, angleRad, maxDistance, outHit);
    }

    inline bool RayCastLayer(glm::vec2 origin, glm::vec2 angleRad, float maxDistance, RayCastHit& outHit, uint16_t layerMask = 0xFFFF, bool includeTriggers = true)
    {
        return Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnRayCastLayer(origin, angleRad, maxDistance, outHit, layerMask, includeTriggers);
    }
    inline bool RayCastLayer(glm::vec2 origin, glm::vec2 angleRad, float maxDistance, RayCastHit& outHit, Core::StateFlags<Physics::PhysicsLayer> layer = Core::StateFlags<Physics::PhysicsLayer>::Full(), bool includeTriggers = true)
    {
        return Online::Runtime::Context::Instance().GetFuncTable<PhysicsSimulator>().InvokeOnRayCastLayer(origin, angleRad, maxDistance, outHit, layer.GetRawValue(), includeTriggers);
    }
}