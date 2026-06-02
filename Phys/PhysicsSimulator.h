#pragma once

#include <box2d/box2d.h>
#include <glm.hpp>
#include <entt/entity/entity.hpp>

#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>
#include <Core/ObjectPool/ObjectPool.h>
#include <Phys/Common/DebugSegment.h>
#include <Phys/Common/ColliderShape.h>
#include <Phys/Common/BodyCreationDesc.h>
#include <Phys/Common/ColliderDesc.h>
#include <Phys/Common/RayCastHit.h>
#include <Phys/Common/Tool/B2BodyIdEqual.h>
#include <Phys/Common/Tool/B2BodyIdHash.h>
#include <Phys/Common/DebugRay.h>

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Online::Physics
{
    class PhysicsSimulator
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<PhysicsSimulator>;
        private:
            static PhysicsSimulator* Create() { return ONLINE_NEW(PhysicsSimulator); }
            static void Destroy(PhysicsSimulator* sim) { ONLINE_DELETE(sim); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<PhysicsSimulator>;
        private:
            static bool Initialize(PhysicsSimulator* sim) { return sim->Initialize(); }
            static void Release(PhysicsSimulator* sim) { sim->Release(); }
            static void FixedUpdate(PhysicsSimulator* sim) { sim->FixedUpdate(); }
        };

    private:
        PhysicsSimulator() = default;
        ~PhysicsSimulator() = default;

    public:
        PhysicsSimulator(const PhysicsSimulator&) = delete;
        PhysicsSimulator& operator=(const PhysicsSimulator&) = delete;
        PhysicsSimulator(PhysicsSimulator&&) = delete;
        PhysicsSimulator& operator=(PhysicsSimulator&&) = delete;

        void FixedUpdate();
        void SyncBodies();

        std::vector<entt::entity> OverlapAABB(glm::vec2 lower, glm::vec2 upper) const;
        std::vector<entt::entity> OverlapCircle(glm::vec2 center, float radius) const;

        std::vector<DebugSegment> GetDebugDrawData() const;

        void RemoveBody(entt::entity entity);
        void SetGravity(glm::vec2 gravity);

        void SubmitBodyCreation(entt::entity entity,
            b2BodyType type = b2_dynamicBody,
            const glm::vec2& position = { 0.0f, 0.0f },
            float angle = 0.0f,
            bool fixedRotation = false,
            float gravityScale = 1.0f,
            float linearDamping = 0.0f,
            float angularDamping = 0.0f,
            bool awake = true);

        void SubmitColliderDesc(entt::entity entity,
            ColliderShape shapeType = ColliderShape::Box,
            const glm::vec2& center = { 0.0f, 0.0f },
            float radius = 0.5f,
            const glm::vec2& halfSize = { 0.5f, 0.5f },
            float angle = 0.0f,
            float density = 1.0f,
            float friction = 0.3f,
            float restitution = 0.0f,
            bool isSensor = false,
            uint64_t categoryBits = 0x0001,
            uint64_t maskBits = 0xFFFF,
            int groupIndex = 0);

        void AddDebugRay(glm::vec2 origin, glm::vec2 direction, float length, glm::vec4 color);

        bool RayCast(glm::vec2 origin, glm::vec2 direction, float maxDistance, RayCastHit& outHit) const;
        bool RayCast(glm::vec2 origin, glm::vec2 direction, float maxDistance, RayCastHit& outHit, uint16_t layerMask, bool includeTriggers = true) const;

        glm::vec2 GetLinearVelocity(entt::entity entity) const;
        void     SetLinearVelocity(entt::entity entity, const glm::vec2& velocity);

        float GetAngularVelocity(entt::entity entity) const;
        void  SetAngularVelocity(entt::entity entity, float omega);

        void ApplyForce(entt::entity entity, const glm::vec2& force);
        void ApplyForceAtPoint(entt::entity entity, const glm::vec2& force, const glm::vec2& worldPoint);
        void ApplyLinearImpulse(entt::entity entity, const glm::vec2& impulse);
        void ApplyLinearImpulseAtPoint(entt::entity entity, const glm::vec2& impulse, const glm::vec2& worldPoint);

        bool IsAwake(entt::entity entity) const;
        void SetAwake(entt::entity entity, bool awake);

    private:
        bool Initialize();
        void Release();

        void FixedUpdate(float unscaledDelta);
        void ConfigureBodyFromDesc(b2BodyId bodyId, const BodyCreationDesc& desc);
        void CreateShape(b2BodyId bodyId, const ColliderDesc& desc);
        void ApplyPhysicsToTransforms();
        void SyncAutoStaticTransforms();

        b2BodyId* AcquireBody();
        void ReleaseBody(b2BodyId* bodyPtr);

        void ProcessFixedUpdateEvent();
        void ProcessSensorEvents();

        static entt::entity BodyUserDataToEntity(void* userData);
        static void* EntityToBodyUserData(entt::entity entity);

        static bool OverlapAABBCallback(b2ShapeId shapeId, void* context);
        static bool OverlapCircleCallback(b2ShapeId shapeId, void* context);

        RayCastHit RayCastTool(glm::vec2 origin, glm::vec2 direction, float maxDistance) const;
        RayCastHit RayCastTool(glm::vec2 origin, glm::vec2 direction, float maxDistance, uint16_t layerMask, bool includeTriggers) const;

        static float RayCastToolCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context);
    private:
        b2WorldId worldId = b2_nullWorldId;
        std::unordered_map<entt::entity, b2BodyId*> entityBodyPtrs;
        std::unordered_map<entt::entity, std::vector<ColliderDesc>> lastColliderDescs;
        std::unordered_set<b2BodyId, B2BodyIdHash, B2BodyIdEqual> allBodies;
        std::unordered_map<entt::entity, b2BodyId*> autoStaticBodyPtrs;

        std::unordered_map<entt::entity, std::unordered_set<entt::entity>> sensorOverlaps;
        std::unordered_map<entt::entity, std::unordered_set<entt::entity>> prevSensorOverlaps;

        float accumulator = 0.0f;
        static constexpr int kMaxStepsPerFrame = 8;
        static constexpr float PPM = 80.0f;
        static constexpr size_t kInitialPoolSize = 512;
        Core::ObjectPool<b2BodyId> bodyPool{ nullptr, nullptr, kInitialPoolSize };
        std::vector<BodyCreationDesc> pendingBodyDescs;
        std::vector<ColliderDesc> pendingColDescs;
        mutable std::vector<DebugRay> debugRays;
    };
}