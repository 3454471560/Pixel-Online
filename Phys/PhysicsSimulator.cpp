#include <Phys/PhysicsSimulator.h>
#include <Time/Common/FuncTable.h>
#include <Phys/Common/Tool/ColliderDescHash.h>
#include <Game/Common/FuncTable.h>
#include <Event/Common/Event.h>
#include <Event/Common/EventType.h>
#include <Event/Common/FuncTable.h>
#include <Event/Common/EventArgs.h>

#include <algorithm>
#include <cstring>
#include <cmath>

namespace Online::Physics
{
    static size_t ComputeColliderSetHash(const std::vector<ColliderDesc>& colliders)
    {
        std::vector<size_t> hashes;
        hashes.reserve(colliders.size());
        for (const auto& cd : colliders)
            hashes.push_back(ColliderDescHash{}(cd));

        std::sort(hashes.begin(), hashes.end());

        size_t combined = 0;
        for (size_t h : hashes)
            combined ^= h + 0x9e3779b9 + (combined << 6) + (combined >> 2);
        return combined;
    }

    entt::entity PhysicsSimulator::BodyUserDataToEntity(void* userData)
    {
        return entt::entity{ static_cast<uint32_t>(reinterpret_cast<uintptr_t>(userData)) };
    }

    void* PhysicsSimulator::EntityToBodyUserData(entt::entity entity)
    {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(entt::to_integral(entity)));
    }

    bool PhysicsSimulator::Initialize()
    {
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = { 0.0f, 9.8f };
        worldId = b2CreateWorld(&worldDef);
        if (!b2World_IsValid(worldId))
            return false;

        for (size_t i = 0; i < kInitialPoolSize; ++i)
        {
            b2BodyId* ptr = bodyPool.Get();
            if (!b2Body_IsValid(*ptr))
            {
                b2BodyDef def = b2DefaultBodyDef();
                def.type = b2_staticBody;
                def.isAwake = false;
                def.position = { 0.0f, 0.0f };
                *ptr = b2CreateBody(worldId, &def);
            }
            bodyPool.Release(ptr);
        }
        return true;
    }

    void PhysicsSimulator::Release()
    {
        // 清理自动静态刚体
        for (auto& [entity, bodyPtr] : autoStaticBodyPtrs)
            ReleaseBody(bodyPtr);
        autoStaticBodyPtrs.clear();

        // 清理显式刚体
        for (auto& [entity, bodyPtr] : entityBodyPtrs)
            ReleaseBody(bodyPtr);
        entityBodyPtrs.clear();

        lastColliderDescs.clear();
        allBodies.clear();
        sensorOverlaps.clear();
        prevSensorOverlaps.clear();

        if (b2World_IsValid(worldId))
        {
            b2DestroyWorld(worldId);
            worldId = b2_nullWorldId;
        }
    }

    void PhysicsSimulator::SetGravity(glm::vec2 gravity)
    {
        if (b2World_IsValid(worldId))
            b2World_SetGravity(worldId, { gravity.x, gravity.y });
    }

    static inline b2Vec2 GlmToB2(glm::vec2 v)
    {
        return { v.x, v.y };
    }

    void PhysicsSimulator::SubmitBodyCreation(entt::entity entity,
        b2BodyType type, const glm::vec2& position, float angle,
        bool fixedRotation, float gravityScale, float linearDamping,
        float angularDamping, bool awake)
    {
        pendingBodyDescs.push_back({ entity, type, position, angle,
            fixedRotation, gravityScale, linearDamping, angularDamping, awake });
    }

    void PhysicsSimulator::SubmitColliderDesc(entt::entity entity,
        ColliderShape shapeType, const glm::vec2& center,
        float radius, const glm::vec2& halfSize, float angle,
        float density, float friction, float restitution, bool isSensor,
        uint64_t categoryBits, uint64_t maskBits, int groupIndex)
    {
        pendingColDescs.push_back({ entity, shapeType, center, radius, halfSize,
            angle, density, friction, restitution, isSensor,
            categoryBits, maskBits, groupIndex });
    }

    void PhysicsSimulator::AddDebugRay(glm::vec2 origin, glm::vec2 direction, float length, glm::vec4 color)
    {
        debugRays.push_back({ origin, direction, length, color });
    }

    bool PhysicsSimulator::RayCast(glm::vec2 origin, glm::vec2 direction, float maxDistance, RayCastHit& outHit) const
    {
        outHit = RayCastTool(origin, direction, maxDistance);
        return outHit.hit;
    }

    bool PhysicsSimulator::RayCast(glm::vec2 origin, glm::vec2 direction, float maxDistance, RayCastHit& outHit,
        uint16_t layerMask, bool includeTriggers) const
    {
        outHit = RayCastTool(origin, direction, maxDistance, layerMask, includeTriggers);
        return outHit.hit;
    }

    void PhysicsSimulator::CreateShape(b2BodyId bodyId, const ColliderDesc& desc)
    {
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = desc.density;
        shapeDef.material.friction = desc.friction;
        shapeDef.material.restitution = desc.restitution;
        shapeDef.isSensor = desc.isSensor;

        b2Filter filter = b2DefaultFilter();
        filter.categoryBits = desc.categoryBits;
        filter.maskBits = desc.maskBits;
        filter.groupIndex = desc.groupIndex;
        shapeDef.filter = filter;

        glm::vec2 finalCenter = desc.offset;
        float finalAngle = desc.angle;

        glm::vec2 posOffset = finalCenter / PPM;
        float halfWidth = desc.halfSize.x / PPM;
        float halfHeight = desc.halfSize.y / PPM;
        float radius = desc.radius / PPM;

        switch (desc.shapeType)
        {
        case ColliderShape::Circle:
        {
            b2Circle circle;
            circle.center = GlmToB2(posOffset);
            circle.radius = radius;
            b2CreateCircleShape(bodyId, &shapeDef, &circle);
            break;
        }
        case ColliderShape::Box:
        {
            b2Rot rot = b2MakeRot(finalAngle);
            b2Polygon box = b2MakeOffsetBox(halfWidth, halfHeight, GlmToB2(posOffset), rot);
            b2CreatePolygonShape(bodyId, &shapeDef, &box);
            break;
        }
        case ColliderShape::Capsule:
        {
            b2Capsule capsule;
            capsule.center1 = GlmToB2(glm::vec2(posOffset.x, posOffset.y - halfHeight));
            capsule.center2 = GlmToB2(glm::vec2(posOffset.x, posOffset.y + halfHeight));
            capsule.radius = radius;
            b2CreateCapsuleShape(bodyId, &shapeDef, &capsule);
            break;
        }
        }
    }

    b2BodyId* PhysicsSimulator::AcquireBody()
    {
        b2BodyId* ptr = bodyPool.Get();
        if (!b2Body_IsValid(*ptr))
        {
            b2BodyDef def = b2DefaultBodyDef();
            def.type = b2_staticBody;
            def.isAwake = false;
            def.position = { 0.0f, 0.0f };
            *ptr = b2CreateBody(worldId, &def);
        }
        return ptr;
    }

    void PhysicsSimulator::ReleaseBody(b2BodyId* bodyPtr)
    {
        if (!bodyPtr || !b2Body_IsValid(*bodyPtr))
            return;

        b2BodyId id = *bodyPtr;

        b2ShapeId shapes[16];
        int count = b2Body_GetShapes(id, shapes, 16);
        for (int i = 0; i < count; ++i)
            b2DestroyShape(shapes[i], false);

        b2Body_SetType(id, b2_staticBody);
        b2Body_SetTransform(id, { 0.0f, 0.0f }, b2Rot_identity);
        b2Body_SetLinearVelocity(id, { 0.0f, 0.0f });
        b2Body_SetAngularVelocity(id, 0.0f);
        b2Body_SetFixedRotation(id, false);
        b2Body_SetGravityScale(id, 1.0f);
        b2Body_SetLinearDamping(id, 0.0f);
        b2Body_SetAngularDamping(id, 0.0f);
        b2Body_SetUserData(id, nullptr);
        b2Body_SetAwake(id, false);

        allBodies.erase(id);
        bodyPool.Release(bodyPtr);
    }

    void PhysicsSimulator::ConfigureBodyFromDesc(b2BodyId bodyId, const BodyCreationDesc& desc)
    {
        b2Body_SetType(bodyId, desc.type);
        b2Body_SetTransform(bodyId,
            { desc.position.x / PPM, desc.position.y / PPM },
            b2MakeRot(desc.angle));
        b2Body_SetFixedRotation(bodyId, desc.fixedRotation);
        b2Body_SetGravityScale(bodyId, desc.gravityScale);
        b2Body_SetLinearDamping(bodyId, desc.linearDamping);
        b2Body_SetAngularDamping(bodyId, desc.angularDamping);
        b2Body_SetUserData(bodyId, EntityToBodyUserData(desc.entity));
        if (desc.awake)
            b2Body_SetAwake(bodyId, true);

        b2ShapeId shapes[16];
        int count = b2Body_GetShapes(bodyId, shapes, 16);
        for (int i = 0; i < count; ++i)
            b2DestroyShape(shapes[i], false);
    }

    void PhysicsSimulator::SyncAutoStaticTransforms()
    {
        for (auto& [entity, bodyPtr] : autoStaticBodyPtrs)
        {
            if (!b2Body_IsValid(*bodyPtr)) continue;

            glm::vec2 worldPos = Online::Game::GetWorldPosition(entity);
            float worldRot = Online::Game::GetWorldRotation(entity);

            b2Body_SetTransform(*bodyPtr,
                { worldPos.x / PPM, worldPos.y / PPM },
                b2MakeRot(worldRot));
        }
    }

    void PhysicsSimulator::SyncBodies()
    {
        if (pendingBodyDescs.empty() && pendingColDescs.empty())
            return;

        std::unordered_map<entt::entity, BodyCreationDesc> bodyMap;
        for (auto& bd : pendingBodyDescs)
            bodyMap[bd.entity] = bd;

        std::unordered_map<entt::entity, std::vector<ColliderDesc>> colliderMap;
        for (auto& cd : pendingColDescs)
            colliderMap[cd.entity].push_back(cd);

        std::vector<entt::entity> entitiesToRemove;
        for (auto& [entity, bodyPtr] : entityBodyPtrs)
        {
            if (bodyMap.find(entity) == bodyMap.end())
                entitiesToRemove.push_back(entity);
        }

        for (auto& [entity, desc] : bodyMap)
        {
            auto autoIt = autoStaticBodyPtrs.find(entity);
            if (autoIt != autoStaticBodyPtrs.end())
            {
                ReleaseBody(autoIt->second);
                autoStaticBodyPtrs.erase(autoIt);
            }

            std::vector<ColliderDesc> newColliders;
            auto cdIt = colliderMap.find(entity);
            if (cdIt != colliderMap.end())
                newColliders = cdIt->second;

            auto it = entityBodyPtrs.find(entity);
            if (it != entityBodyPtrs.end())
            {
                b2BodyId* oldBodyPtr = it->second;
                b2BodyId oldBody = *oldBodyPtr;
                b2BodyType oldType = b2Body_GetType(oldBody);
                bool oldFixedRotation = b2Body_IsFixedRotation(oldBody);

                std::vector<ColliderDesc> oldColliders;
                auto lastIt = lastColliderDescs.find(entity);
                if (lastIt != lastColliderDescs.end())
                    oldColliders = lastIt->second;

                bool needRebuild = (oldType != desc.type) || (oldFixedRotation != desc.fixedRotation);
                if (!needRebuild)
                {
                    if (oldColliders.size() != newColliders.size())
                        needRebuild = true;
                    else
                    {
                        size_t oldHash = ComputeColliderSetHash(oldColliders);
                        size_t newHash = ComputeColliderSetHash(newColliders);
                        if (oldHash != newHash)
                            needRebuild = true;
                    }
                }

                if (needRebuild)
                {
                    b2Vec2 oldVel = b2Body_GetLinearVelocity(oldBody);
                    float oldAngVel = b2Body_GetAngularVelocity(oldBody);

                    ReleaseBody(oldBodyPtr);

                    b2BodyId* newBodyPtr = AcquireBody();
                    b2BodyId newBody = *newBodyPtr;
                    ConfigureBodyFromDesc(newBody, desc);
                    b2Body_SetLinearVelocity(newBody, oldVel);
                    b2Body_SetAngularVelocity(newBody, oldAngVel);

                    for (auto& cd : newColliders)
                        CreateShape(newBody, cd);

                    entityBodyPtrs[entity] = newBodyPtr;
                    allBodies.insert(newBody);
                }
                else
                {
                    if (desc.type == b2_kinematicBody)
                    {
                        b2Vec2 physPos = { desc.position.x / PPM, desc.position.y / PPM };
                        b2Body_SetTransform(oldBody, physPos, b2MakeRot(desc.angle));
                    }
                    b2Body_SetGravityScale(oldBody, desc.gravityScale);
                    b2Body_SetLinearDamping(oldBody, desc.linearDamping);
                    b2Body_SetAngularDamping(oldBody, desc.angularDamping);
                    if (desc.awake)
                        b2Body_SetAwake(oldBody, true);
                }
            }
            else
            {
                b2BodyId* newBodyPtr = AcquireBody();
                b2BodyId newBody = *newBodyPtr;
                ConfigureBodyFromDesc(newBody, desc);

                for (auto& cd : newColliders)
                    CreateShape(newBody, cd);

                entityBodyPtrs[entity] = newBodyPtr;
                allBodies.insert(newBody);
            }

            if (!newColliders.empty())
                lastColliderDescs[entity] = newColliders;
            else
                lastColliderDescs.erase(entity);
        }

        for (auto& [entity, newColliders] : colliderMap)
        {
            if (bodyMap.find(entity) != bodyMap.end() || entityBodyPtrs.find(entity) != entityBodyPtrs.end())
                continue;

            auto autoIt = autoStaticBodyPtrs.find(entity);
            if (autoIt != autoStaticBodyPtrs.end())
            {
                std::vector<ColliderDesc> oldColliders;
                auto lastIt = lastColliderDescs.find(entity);
                if (lastIt != lastColliderDescs.end())
                    oldColliders = lastIt->second;

                bool needRebuild = (oldColliders.size() != newColliders.size());
                if (!needRebuild)
                {
                    size_t oldHash = ComputeColliderSetHash(oldColliders);
                    size_t newHash = ComputeColliderSetHash(newColliders);
                    needRebuild = (oldHash != newHash);
                }

                if (needRebuild)
                {
                    ReleaseBody(autoIt->second);
                    autoStaticBodyPtrs.erase(autoIt);

                    b2BodyId* newBodyPtr = AcquireBody();
                    b2BodyId newBody = *newBodyPtr;
                    b2Body_SetType(newBody, b2_staticBody);
                    b2Body_SetUserData(newBody, EntityToBodyUserData(entity));

                    glm::vec2 worldPos = Online::Game::GetWorldPosition(entity);
                    float worldRot = Online::Game::GetWorldRotation(entity);
                    b2Body_SetTransform(newBody, { worldPos.x / PPM, worldPos.y / PPM }, b2MakeRot(worldRot));

                    for (auto& cd : newColliders)
                        CreateShape(newBody, cd);

                    autoStaticBodyPtrs[entity] = newBodyPtr;
                    allBodies.insert(newBody);
                }
            }
            else
            {
                b2BodyId* newBodyPtr = AcquireBody();
                b2BodyId newBody = *newBodyPtr;
                b2Body_SetType(newBody, b2_staticBody);
                b2Body_SetUserData(newBody, EntityToBodyUserData(entity));

                glm::vec2 worldPos = Online::Game::GetWorldPosition(entity);
                float worldRot = Online::Game::GetWorldRotation(entity);
                b2Body_SetTransform(newBody, { worldPos.x / PPM, worldPos.y / PPM }, b2MakeRot(worldRot));

                for (auto& cd : newColliders)
                    CreateShape(newBody, cd);

                autoStaticBodyPtrs[entity] = newBodyPtr;
                allBodies.insert(newBody);
            }

            if (!newColliders.empty())
                lastColliderDescs[entity] = newColliders;
            else
                lastColliderDescs.erase(entity);
        }

        for (entt::entity entity : entitiesToRemove)
        {
            auto it = entityBodyPtrs.find(entity);
            if (it != entityBodyPtrs.end())
            {
                ReleaseBody(it->second);
                entityBodyPtrs.erase(it);
                lastColliderDescs.erase(entity);
            }
        }

        pendingBodyDescs.clear();
        pendingColDescs.clear();
    }

    void PhysicsSimulator::ProcessFixedUpdateEvent()
    {
        Online::Event::PhysFixedUpdateEventArgs args;

        Online::Event::Emit(Online::Event::Event(
            Online::Event::EventType::PhysFixedUpdate, &args));
    }

    void PhysicsSimulator::ProcessSensorEvents()
    {
        auto oldOverlaps = std::move(prevSensorOverlaps);
        prevSensorOverlaps.clear();

        std::unordered_map<entt::entity, std::unordered_set<entt::entity>> newSensorOverlaps;

        std::vector<b2BodyId> sensorBodies;
        for (b2BodyId bodyId : allBodies)
        {
            if (!b2Body_IsValid(bodyId))
                continue;

            b2ShapeId shapes[64];
            int count = b2Body_GetShapes(bodyId, shapes, 64);
            for (int i = 0; i < count; ++i)
            {
                if (b2Shape_IsSensor(shapes[i]))
                {
                    sensorBodies.push_back(bodyId);
                    break;
                }
            }
        }

        for (b2BodyId sensorBody : sensorBodies)
        {
            entt::entity sensorEntity = BodyUserDataToEntity(b2Body_GetUserData(sensorBody));
            if (sensorEntity == entt::null)
                continue;

            b2AABB aabb = b2Body_ComputeAABB(sensorBody);

            std::vector<entt::entity> entities;
            b2QueryFilter filter = b2DefaultQueryFilter();
            b2World_OverlapAABB(worldId, aabb, filter, OverlapAABBCallback, &entities);

            for (entt::entity other : entities)
            {
                if (other == sensorEntity)
                    continue;
                newSensorOverlaps[sensorEntity].insert(other);
                newSensorOverlaps[other].insert(sensorEntity);
            }
        }

        for (const auto& [entity, newSet] : newSensorOverlaps)
        {
            const auto& oldSet = oldOverlaps[entity];
            for (entt::entity other : newSet)
            {
                if (oldSet.find(other) == oldSet.end())
                {
                    Online::Event::PhysTriggerEventArgs args(entity, other);
                    Online::Event::Emit(Online::Event::Event(
                        Online::Event::EventType::PhysicsTriggerEnter, &args));
                }
            }
        }

        for (const auto& [entity, oldSet] : oldOverlaps)
        {
            const auto& newSet = newSensorOverlaps[entity];
            for (entt::entity other : oldSet)
            {
                if (newSet.find(other) == newSet.end())
                {
                    Online::Event::PhysTriggerEventArgs args(entity, other);
                    Online::Event::Emit(Online::Event::Event(
                        Online::Event::EventType::PhysicsTriggerExit, &args));
                }
            }
        }

        for (const auto& [entity, set] : newSensorOverlaps)
        {
            for (entt::entity other : set)
            {
                Online::Event::PhysTriggerEventArgs args(entity, other);
                Online::Event::Emit(Online::Event::Event(
                    Online::Event::EventType::PhysicsTriggerStay, &args));
            }
        }

        sensorOverlaps = newSensorOverlaps;
        prevSensorOverlaps = std::move(newSensorOverlaps);
    }

    void PhysicsSimulator::FixedUpdate()
    {
        FixedUpdate(Time::unscaledDelta());
    }

    void PhysicsSimulator::FixedUpdate(float unscaledDelta)
    {
        accumulator += unscaledDelta;
        accumulator = std::min(accumulator, Time::fixdelta() * kMaxStepsPerFrame);

        SyncAutoStaticTransforms();

        while (accumulator >= Time::fixdelta())
        {
            ProcessFixedUpdateEvent();
            b2World_Step(worldId, Time::fixdelta(), 10);
            accumulator -= Time::fixdelta();
        }

        ProcessSensorEvents();

        ApplyPhysicsToTransforms();
    }

    void PhysicsSimulator::ApplyPhysicsToTransforms()
    {
        for (auto& [entity, bodyPtr] : entityBodyPtrs)
        {
            b2BodyId body = *bodyPtr;
            if (!b2Body_IsValid(body)) continue;
            if (b2Body_GetType(body) == b2_staticBody) continue;

            b2Vec2 pos = b2Body_GetPosition(body);
            b2Rot rot = b2Body_GetRotation(body);
            float angle = b2Rot_GetAngle(rot);

            Online::Game::TransformUpdater(entity, glm::vec2(pos.x * PPM, pos.y * PPM), angle);
        }
    }

    bool PhysicsSimulator::OverlapAABBCallback(b2ShapeId shapeId, void* context)
    {
        auto* entities = static_cast<std::vector<entt::entity>*>(context);
        b2BodyId bodyId = b2Shape_GetBody(shapeId);
        void* ud = b2Body_GetUserData(bodyId);
        entt::entity e = BodyUserDataToEntity(ud);
        if (e != entt::null)
            entities->push_back(e);
        return true;
    }

    bool PhysicsSimulator::OverlapCircleCallback(b2ShapeId shapeId, void* context)
    {
        auto* entities = static_cast<std::vector<entt::entity>*>(context);
        b2BodyId bodyId = b2Shape_GetBody(shapeId);
        void* ud = b2Body_GetUserData(bodyId);
        entt::entity e = BodyUserDataToEntity(ud);
        if (e != entt::null)
            entities->push_back(e);
        return true;
    }

    RayCastHit PhysicsSimulator::RayCastTool(glm::vec2 origin, glm::vec2 direction,float maxDistance) const
    {
        b2Vec2 rayStart = { origin.x / PPM, origin.y / PPM };
        b2Vec2 rayEnd = { (origin.x + direction.x * maxDistance) / PPM,
                          (origin.y + direction.y * maxDistance) / PPM };
        b2Vec2 translation = { rayEnd.x - rayStart.x, rayEnd.y - rayStart.y };
        b2QueryFilter filter = b2DefaultQueryFilter();
        b2RayResult result = b2World_CastRayClosest(worldId, rayStart, translation, filter);

        RayCastHit hit;
        if (result.hit)
        {
            hit.hit = true;
            b2BodyId bodyId = b2Shape_GetBody(result.shapeId);
            hit.entity = BodyUserDataToEntity(b2Body_GetUserData(bodyId));
            hit.point = glm::vec2(result.point.x * PPM, result.point.y * PPM);
            hit.distance = glm::distance(hit.point, origin);
            hit.normal = glm::vec2(result.normal.x, result.normal.y);
            hit.fraction = result.fraction;
        }
        return hit;
    }

    float PhysicsSimulator::RayCastToolCallback(b2ShapeId shapeId, b2Vec2 point, b2Vec2 normal, float fraction, void* context)
    {
        // context 是一个结构体，包含我们需要的信息
        struct CallbackContext {
            std::vector<b2ShapeId> shapeIds;
            std::vector<b2Vec2> points;
            std::vector<b2Vec2> normals;
            std::vector<float> fractions;
            bool includeTriggers;
            uint16_t layerMask;
        };
        auto* ctx = static_cast<CallbackContext*>(context);

        // 触发器过滤
        if (!ctx->includeTriggers && b2Shape_IsSensor(shapeId))
            return 0.0f;  // 忽略该交点

        // 层级二次确认（b2QueryFilter 已做初步过滤，这里可留也可不写，但保留以防万一）
        b2Filter shapeFilter = b2Shape_GetFilter(shapeId);
        if ((shapeFilter.categoryBits & ctx->layerMask) == 0)
            return 0.0f;

        // 记录交点
        ctx->shapeIds.push_back(shapeId);
        ctx->points.push_back(point);
        ctx->normals.push_back(normal);
        ctx->fractions.push_back(fraction);

        return fraction;  // 返回 fraction 表示接受此交点
    }

    RayCastHit PhysicsSimulator::RayCastTool(glm::vec2 origin, glm::vec2 direction,
        float maxDistance, uint16_t layerMask,
        bool includeTriggers) const
    {
        // 转换坐标
        b2Vec2 rayStart = { origin.x / PPM, origin.y / PPM };
        b2Vec2 rayEnd = { (origin.x + direction.x * maxDistance) / PPM,
                          (origin.y + direction.y * maxDistance) / PPM };
        b2Vec2 translation = { rayEnd.x - rayStart.x, rayEnd.y - rayStart.y };

        // 构建过滤器
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = layerMask;          // 只接受 categoryBits 与 layerMask 有交集的形状
        filter.categoryBits = UINT64_MAX;     // 射线属于所有类别

        // 准备回调上下文
        struct CallbackContext {
            std::vector<b2ShapeId> shapeIds;
            std::vector<b2Vec2> points;
            std::vector<b2Vec2> normals;
            std::vector<float> fractions;
            bool includeTriggers;
            uint16_t layerMask;
        } ctx;
        ctx.includeTriggers = includeTriggers;
        ctx.layerMask = layerMask;

        b2World_CastRay(worldId, rayStart, translation, filter, RayCastToolCallback, &ctx);

        // 从所有命中中找出最近的
        RayCastHit closestHit{};
        float closestFraction = FLT_MAX;
        for (size_t i = 0; i < ctx.shapeIds.size(); ++i)
        {
            if (ctx.fractions[i] < closestFraction)
            {
                closestFraction = ctx.fractions[i];
                closestHit.hit = true;
                b2BodyId bodyId = b2Shape_GetBody(ctx.shapeIds[i]);
                closestHit.entity = BodyUserDataToEntity(b2Body_GetUserData(bodyId));
                closestHit.point = glm::vec2(ctx.points[i].x * PPM, ctx.points[i].y * PPM);
                closestHit.distance = glm::distance(closestHit.point, origin);
                closestHit.normal = glm::vec2(ctx.normals[i].x, ctx.normals[i].y);
                closestHit.fraction = ctx.fractions[i];
            }
        }
        return closestHit;
    }

    std::vector<entt::entity> PhysicsSimulator::OverlapAABB(glm::vec2 lower, glm::vec2 upper) const
    {
        std::vector<entt::entity> entities;
        b2AABB aabb{ { lower.x / PPM, lower.y / PPM }, { upper.x / PPM, upper.y / PPM } };
        b2QueryFilter filter = b2DefaultQueryFilter();
        b2World_OverlapAABB(worldId, aabb, filter, OverlapAABBCallback, &entities);
        return entities;
    }

    std::vector<entt::entity> PhysicsSimulator::OverlapCircle(glm::vec2 center, float radius) const
    {
        std::vector<entt::entity> entities;
        b2Vec2 point = { center.x / PPM, center.y / PPM };
        float r = radius / PPM;
        b2ShapeProxy proxy = b2MakeProxy(&point, 1, r);
        b2QueryFilter filter = b2DefaultQueryFilter();
        b2World_OverlapShape(worldId, &proxy, filter, OverlapCircleCallback, &entities);
        return entities;
    }

    std::vector<DebugSegment> PhysicsSimulator::GetDebugDrawData() const
    {
        std::vector<DebugSegment> segments;

        for (b2BodyId bodyId : allBodies)
        {
            if (!b2Body_IsValid(bodyId)) continue;

            b2Vec2 bodyPos = b2Body_GetPosition(bodyId);
            b2Rot bodyRot = b2Body_GetRotation(bodyId);
            b2Transform bodyTransform{ bodyPos, bodyRot };

            const int maxShapes = 128;
            b2ShapeId shapes[maxShapes];
            int shapeCount = b2Body_GetShapes(bodyId, shapes, maxShapes);
            for (int j = 0; j < shapeCount; ++j)
            {
                b2ShapeId shapeId = shapes[j];
                b2ShapeType type = b2Shape_GetType(shapeId);
                glm::vec4 color = (type == b2_circleShape) ?
                    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) :
                    glm::vec4(0.0f, 0.5f, 1.0f, 1.0f);

                if (type == b2_polygonShape)
                {
                    b2Polygon poly = b2Shape_GetPolygon(shapeId);
                    b2Vec2 worldVerts[B2_MAX_POLYGON_VERTICES];
                    for (int k = 0; k < poly.count; ++k)
                        worldVerts[k] = b2TransformPoint(bodyTransform, poly.vertices[k]);

                    for (int k = 0; k < poly.count; ++k)
                    {
                        b2Vec2 p1 = worldVerts[k];
                        b2Vec2 p2 = worldVerts[(k + 1) % poly.count];
                        segments.push_back({ glm::vec2(p1.x * PPM, p1.y * PPM),
                                             glm::vec2(p2.x * PPM, p2.y * PPM), color });
                    }
                }
                else if (type == b2_circleShape)
                {
                    b2Circle circle = b2Shape_GetCircle(shapeId);
                    glm::vec2 center = glm::vec2(bodyPos.x, bodyPos.y) + glm::vec2(circle.center.x, circle.center.y);
                    float r = circle.radius;
                    const int segCount = 24;
                    for (int k = 0; k < segCount; ++k)
                    {
                        float a1 = 2.0f * 3.14159265f * k / segCount;
                        float a2 = 2.0f * 3.14159265f * (k + 1) / segCount;
                        glm::vec2 p1 = center + r * glm::vec2(cosf(a1), sinf(a1));
                        glm::vec2 p2 = center + r * glm::vec2(cosf(a2), sinf(a2));
                        segments.push_back({ glm::vec2(p1.x * PPM, p1.y * PPM),
                                             glm::vec2(p2.x * PPM, p2.y * PPM), color });
                    }
                }
                else if (type == b2_capsuleShape)
                {
                    b2Capsule capsule = b2Shape_GetCapsule(shapeId);
                    const int segCount = 12;
                    float r = capsule.radius;

                    b2Vec2 center1 = b2TransformPoint(bodyTransform, capsule.center1);
                    b2Vec2 center2 = b2TransformPoint(bodyTransform, capsule.center2);

                    glm::vec2 wCenter1 = { center1.x, center1.y };
                    glm::vec2 wCenter2 = { center2.x, center2.y };
                    glm::vec2 dir = glm::normalize(wCenter2 - wCenter1);
                    glm::vec2 normal = { -dir.y, dir.x };

                    auto rotatePoint = [&](float angle, const glm::vec2& pivot) -> glm::vec2
                        {
                            return pivot + normal * cosf(angle) * r + dir * sinf(angle) * r;
                        };

                    for (int k = 0; k < segCount; ++k)
                    {
                        float a1 = 3.14159265f * (1.0f + (float)k / segCount);
                        float a2 = 3.14159265f * (1.0f + (float)(k + 1) / segCount);
                        glm::vec2 p1 = rotatePoint(a1, wCenter1);
                        glm::vec2 p2 = rotatePoint(a2, wCenter1);
                        segments.push_back({ glm::vec2(p1.x * PPM, p1.y * PPM),
                                             glm::vec2(p2.x * PPM, p2.y * PPM), color });
                    }

                    for (int k = 0; k < segCount; ++k)
                    {
                        float a1 = 3.14159265f * (float)k / segCount;
                        float a2 = 3.14159265f * (float)(k + 1) / segCount;
                        glm::vec2 p1 = rotatePoint(a1, wCenter2);
                        glm::vec2 p2 = rotatePoint(a2, wCenter2);
                        segments.push_back({ glm::vec2(p1.x * PPM, p1.y * PPM),
                                             glm::vec2(p2.x * PPM, p2.y * PPM), color });
                    }

                    segments.push_back({ glm::vec2((wCenter1.x + normal.x * r) * PPM, (wCenter1.y + normal.y * r) * PPM),
                                         glm::vec2((wCenter2.x + normal.x * r) * PPM, (wCenter2.y + normal.y * r) * PPM), color });
                    segments.push_back({ glm::vec2((wCenter1.x - normal.x * r) * PPM, (wCenter1.y - normal.y * r) * PPM),
                                         glm::vec2((wCenter2.x - normal.x * r) * PPM, (wCenter2.y - normal.y * r) * PPM), color });
                }
            }
        }

        for (const auto& ray : debugRays)
        {
            glm::vec2 end = ray.origin + ray.direction * ray.length;
            segments.push_back({ ray.origin, end, ray.color });
        }
        debugRays.clear();

        return segments;
    }

    void PhysicsSimulator::RemoveBody(entt::entity entity)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end())
        {
            ReleaseBody(it->second);
            entityBodyPtrs.erase(it);
        }

        auto autoIt = autoStaticBodyPtrs.find(entity);
        if (autoIt != autoStaticBodyPtrs.end())
        {
            ReleaseBody(autoIt->second);
            autoStaticBodyPtrs.erase(autoIt);
            lastColliderDescs.erase(entity);
        }

        auto soIt = sensorOverlaps.find(entity);
        if (soIt != sensorOverlaps.end())
        {
            for (entt::entity other : soIt->second)
            {
                auto otherIt = sensorOverlaps.find(other);
                if (otherIt != sensorOverlaps.end())
                    otherIt->second.erase(entity);
            }
            sensorOverlaps.erase(soIt);
        }
        auto psoIt = prevSensorOverlaps.find(entity);
        if (psoIt != prevSensorOverlaps.end())
        {
            for (entt::entity other : psoIt->second)
            {
                auto otherIt = prevSensorOverlaps.find(other);
                if (otherIt != prevSensorOverlaps.end())
                    otherIt->second.erase(entity);
            }
            prevSensorOverlaps.erase(psoIt);
        }
    }

    glm::vec2 PhysicsSimulator::GetLinearVelocity(entt::entity entity) const
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
        {
            b2Vec2 v = b2Body_GetLinearVelocity(*it->second);
            return { v.x, v.y };
        }
        return { 0.0f, 0.0f };
    }

    void PhysicsSimulator::SetLinearVelocity(entt::entity entity, const glm::vec2& velocity)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
        {
            b2Body_SetLinearVelocity(*it->second, { velocity.x, velocity.y });
            b2Body_SetAwake(*it->second, true);
        }
    }

    float PhysicsSimulator::GetAngularVelocity(entt::entity entity) const
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
            return b2Body_GetAngularVelocity(*it->second);
        return 0.0f;
    }

    void PhysicsSimulator::SetAngularVelocity(entt::entity entity, float omega)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
        {
            b2Body_SetAngularVelocity(*it->second, omega);
            b2Body_SetAwake(*it->second, true);
        }
    }

    void PhysicsSimulator::ApplyForce(entt::entity entity, const glm::vec2& force)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
            b2Body_ApplyForceToCenter(*it->second, { force.x, force.y }, true);
    }

    void PhysicsSimulator::ApplyForceAtPoint(entt::entity entity, const glm::vec2& force, const glm::vec2& worldPoint)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
            b2Body_ApplyForce(*it->second, { force.x, force.y }, { worldPoint.x, worldPoint.y }, true);
    }

    void PhysicsSimulator::ApplyLinearImpulse(entt::entity entity, const glm::vec2& impulse)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
            b2Body_ApplyLinearImpulseToCenter(*it->second, { impulse.x, impulse.y }, true);
    }

    void PhysicsSimulator::ApplyLinearImpulseAtPoint(entt::entity entity, const glm::vec2& impulse, const glm::vec2& worldPoint)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
            b2Body_ApplyLinearImpulse(*it->second, { impulse.x, impulse.y }, { worldPoint.x, worldPoint.y }, true);
    }

    bool PhysicsSimulator::IsAwake(entt::entity entity) const
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
            return b2Body_IsAwake(*it->second);
        return false;
    }

    void PhysicsSimulator::SetAwake(entt::entity entity, bool awake)
    {
        auto it = entityBodyPtrs.find(entity);
        if (it != entityBodyPtrs.end() && b2Body_IsValid(*it->second))
            b2Body_SetAwake(*it->second, awake);
    }
}