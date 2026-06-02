#pragma once
#include <Core/StateFlags/StateFlags.h>
#include <Phys/Common/ColliderShape.h>
#include <Phys/Common/PhysicsLayer.h>
#include <Game/Component/Component.h>
#include <Game/Common/FuncTable.h>

namespace Online::Game
{
    struct Collider : public Component 
    {
    public:
        inline void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("shape", static_cast<int>(shape));
            ctx.Write("center", offset);
            ctx.Write("radius", radius);
            ctx.Write("halfSize", halfSize);
            ctx.Write("angle", angle);

            ctx.Write("density", density);
            ctx.Write("friction", friction);
            ctx.Write("restitution", restitution);
            ctx.Write("isSensor", isTrigger);

            ctx.Write("categoryBits", categoryBits.GetRawValue());
            ctx.Write("maskBits", maskBits.GetRawValue());
            ctx.Write("groupIndex", static_cast<int>(groupIndex));
        }

        void Deserialize(const Serialize::DeserializeContext& ctx) override
        {
            int s = static_cast<int>(shape);
            if (ctx.Read("shape", s))
            {
                shape = static_cast<Online::Physics::ColliderShape>(s);
            }

            ctx.Read("center", offset);
            ctx.Read("radius", radius);
            ctx.Read("halfSize", halfSize);
            ctx.Read("angle", angle);

            ctx.Read("density", density);
            ctx.Read("friction", friction);
            ctx.Read("restitution", restitution);
            ctx.Read("isSensor", isTrigger);

            uint16_t catRaw{};
            if (ctx.Read("categoryBits", catRaw))
            {
                Core::StateFlags<Physics::PhysicsLayer> tempFlag;
                tempFlag.OverwriteBits(static_cast<Physics::PhysicsLayer>(catRaw));
                SetCategory(tempFlag);
            }
            uint16_t maskRaw{};
            if (ctx.Read("maskBits", maskRaw))
            {
                Core::StateFlags<Physics::PhysicsLayer> tempFlag;
                tempFlag.OverwriteBits(static_cast<Physics::PhysicsLayer>(maskRaw));
                SetMask(tempFlag);
            }

            int gi = groupIndex;
            if (ctx.Read("groupIndex", gi))
            {
                groupIndex = static_cast<int16_t>(gi);
            }
        }
    public:
        inline Online::Physics::ColliderShape GetShape() const { return shape; }
        inline void SetShape(Online::Physics::ColliderShape s) { shape = s; }

        inline const glm::vec2& GetOffset() const { return offset + ExtraOffset; }
        inline const glm::vec2& GetBaseOffset() const { return offset; }
        inline const glm::vec2& GetExtraOffset() const { return ExtraOffset; }
        inline void SetOffset(const glm::vec2& c) { offset = c; }
        inline void SetExtraOffset(const glm::vec2& c) { ExtraOffset = c; }


        inline float GetRadius() const { return radius; }
        inline void SetRadius(float r) { radius = r; }

        inline const glm::vec2& GetHalfSize() const { return halfSize; }
        inline void SetHalfSize(const glm::vec2& hs) { halfSize = hs; }

        inline float GetAngle() const { return angle + ExtraAngle; }
        inline float GetBaseAngle() const { return angle; }
        inline float GetExtraAngle() const { return ExtraAngle; }
        inline void SetAngle(float a) { angle = a; }
        inline void SetExtraAngle(float a) { ExtraAngle = a; }

        inline float GetDensity() const { return density; }
        inline void SetDensity(float d) { density = d; }

        inline float GetFriction() const { return friction; }
        inline void SetFriction(float f) { friction = f; }

        inline float GetRestitution() const { return restitution; }
        inline void SetRestitution(float r) { restitution = r; }

        inline bool IsTrigger() const { return isTrigger; }
        inline void SetTrigger(bool s) { isTrigger = s; }

        inline uint16_t GetCategoryBits() const 
        { 
            return categoryBits.GetRawValue(); 
        }

        inline void AddCategoryBits(Physics::PhysicsLayer layer)
        {
            categoryBits.SetBits(layer);
        }

        inline void RemoveCategoryBits(Physics::PhysicsLayer layer)
        {
            categoryBits.ClearBits(layer);
        }

        inline void SetCategory(Core::StateFlags<Physics::PhysicsLayer> layer)
        {
            categoryBits.OverwriteFrom(layer);
        }

        inline void SetCategoryBits(Physics::PhysicsLayer layer)
        {
            categoryBits.ClearBits(layer);
            categoryBits.SetBits(layer);
        }

        inline uint16_t GetMaskBits() const 
        { 
            return maskBits.GetRawValue();
        }

        inline void AddMaskBits(Physics::PhysicsLayer layer)
        { 
            maskBits.SetBits(layer);
        }

        inline void RemoveMaskBits(Physics::PhysicsLayer layer)
        { 
            maskBits.ClearBits(layer);
        }

        inline void SetMask(Core::StateFlags<Physics::PhysicsLayer> layer)
        {
            maskBits.OverwriteFrom(layer);
        }

        inline void SetMaskBits(Physics::PhysicsLayer layer)
        {
            maskBits.ClearBits(layer);
            maskBits.SetBits(layer);
        }

        inline int16_t GetGroupIndex() const { return groupIndex; }
        inline void SetGroupIndex(int16_t idx) { groupIndex = idx; }

        inline void SetRigidEntity(entt::entity entity)
        {
            RigidEntity = entity;
        }
        inline entt::entity GetRigidEntity() const
        {
            return RigidEntity;
		}

        inline bool IsRigidReady() const
        {
            return RigidReady;
		}
        inline void SetRigidReady(bool flag)
        {
            RigidReady = flag;
		}
    private:
        Online::Physics::ColliderShape shape = Online::Physics::ColliderShape::Box;
        glm::vec2 offset = { 0,0 }; 
        glm::vec2 ExtraOffset = { 0,0 };
        float radius = 40.0f;
        glm::vec2 halfSize = { 40.0f, 40.0f };
        float angle = 0.0f;
        float ExtraAngle = 0.0f;
        float density = 1.0f;
        float friction = 0.3f;
        float restitution = 0.0f;
        bool isTrigger = false;
        Core::StateFlags<Physics::PhysicsLayer> categoryBits = Physics::PhysicsLayer::Default;
        Core::StateFlags<Physics::PhysicsLayer> maskBits = Core::StateFlags<Physics::PhysicsLayer>::Full();
        int16_t groupIndex = 0;
        entt::entity RigidEntity = entt::null;
        bool RigidReady = false;
    };
}