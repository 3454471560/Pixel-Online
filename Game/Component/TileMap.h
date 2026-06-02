#pragma once
#include <Asset/Common/ID/TextureID.h>
#include <Render/Common/RenderQueue.h>
#include <Config/Common/TileMapID.h>
#include <Game/Component/Collider.h>
#include <Game/Component/Component.h>
#include <Phys/Common/BodyType.h>
#include <Config/Common/FuncTable.h>
#include <vector>
#include <unordered_set>
#include <glm.hpp>

namespace Online::Game
{
    class GameObject;

    struct TileMap : public Component
    {
    public:
        // ---------- 刚体属性 ----------
        inline Online::Physics::BodyType GetBodyType() const { return bodyType; }
        inline void SetBodyType(Online::Physics::BodyType t) { bodyType = t; }

        inline bool IsFixedRotation() const { return fixedRotation; }
        inline void SetFixedRotation(bool v) { fixedRotation = v; }

        inline float GetGravityScale() const { return gravityScale; }
        inline void SetGravityScale(float v) { gravityScale = v; }

        inline float GetLinearDamping() const { return linearDamping; }
        inline void SetLinearDamping(float v) { linearDamping = v; }

        inline float GetAngularDamping() const { return angularDamping; }
        inline void SetAngularDamping(float v) { angularDamping = v; }

        inline bool IsAwake() const { return awake; }
        inline void SetAwake(bool v) { awake = v; }

        inline const glm::vec2& GetColliderOffset() const { return colliderOffset; }
        inline void SetColliderOffset(const glm::vec2& offset)
        {
            colliderOffset = offset;
            RefreshColliders();
        }

        Collider* GetCollider(size_t idx)
        {
            if (idx >= colliders.size()) return nullptr;
            return &colliders[idx];
        }
        const Collider* GetCollider(size_t idx) const
        {
            if (idx >= colliders.size()) return nullptr;
            return &colliders[idx];
        }

        size_t GetColliderCount() const { return colliders.size(); }

        Collider* begin() { return colliders.data(); }
        Collider* end() { return colliders.data() + colliders.size(); }
        const Collider* begin() const { return colliders.data(); }
        const Collider* end()   const { return colliders.data() + colliders.size(); }

        void RefreshColliders()
        {
            const auto& tileConfig = Online::Config::GetTileMap(ID);
            std::vector<glm::ivec2> indices = tileConfig.GetTerrainTileIndices();
            colliders.clear();
            if (indices.empty()) return;

            // 自定义哈希
            struct IVec2Hash {
                size_t operator()(const glm::ivec2& v) const {
                    return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
                }
            };
            std::unordered_set<glm::ivec2, IVec2Hash> tileSet;
            for (auto& idx : indices) tileSet.insert(idx);

            std::unordered_set<glm::ivec2, IVec2Hash> visited;
            std::vector<std::pair<glm::ivec2, glm::ivec2>> rectangles; // (min, max)

            for (const auto& idx : indices) {
                if (visited.count(idx)) continue;
                int minX = idx.x, maxX = idx.x;
                int minY = idx.y, maxY = idx.y;

                // 向右扩展
                while (tileSet.count({ maxX + 1, idx.y }) && !visited.count({ maxX + 1, idx.y }))
                    ++maxX;

                // 向下扩展
                bool extend = true;
                while (extend) {
                    for (int x = minX; x <= maxX; ++x) {
                        if (!tileSet.count({ x, maxY + 1 }) || visited.count({ x, maxY + 1 })) {
                            extend = false;
                            break;
                        }
                    }
                    if (extend) ++maxY;
                }

                // 标记已访问
                for (int x = minX; x <= maxX; ++x)
                    for (int y = minY; y <= maxY; ++y)
                        visited.insert({ x, y });

                rectangles.emplace_back(glm::ivec2(minX, minY), glm::ivec2(maxX, maxY));
            }

            colliders.reserve(rectangles.size());
            for (auto& [min, max] : rectangles) {
                float left = min.x * 32.0f;
                float bottom = min.y * 32.0f;
                float right = (max.x + 1) * 32.0f;
                float top = (max.y + 1) * 32.0f;

                glm::vec2 center = glm::vec2((left + right) * 0.5f, (bottom + top) * 0.5f) + colliderOffset;
                glm::vec2 halfSize = glm::vec2((right - left) * 0.5f, (top - bottom) * 0.5f);

                Collider col;
                col.SetShape(Online::Physics::ColliderShape::Box);
                col.SetOffset(center);
                col.SetHalfSize(halfSize);
                col.SetDensity(templateDensity);
                col.SetFriction(templateFriction);
                col.SetRestitution(templateRestitution);
                col.SetTrigger(templateIsTrigger);
                col.SetCategory(templateCategoryBits);
                col.SetMask(templateMaskBits);
                col.SetGroupIndex(templateGroupIndex);
                colliders.push_back(col);
            }
        }

        inline void SetTemplateDensity(float d) { templateDensity = d; RefreshColliders(); }
        inline float GetTemplateDensity() const { return templateDensity; }

        inline void SetTemplateFriction(float f) { templateFriction = f; RefreshColliders(); }
        inline float GetTemplateFriction() const { return templateFriction; }

        inline void SetTemplateRestitution(float r) { templateRestitution = r; RefreshColliders(); }
        inline float GetTemplateRestitution() const { return templateRestitution; }

        inline void SetTemplateIsTrigger(bool t) { templateIsTrigger = t; RefreshColliders(); }
        inline bool GetTemplateIsTrigger() const { return templateIsTrigger; }

        inline Core::StateFlags<Physics::PhysicsLayer> GetTemplateCategoryBits() const { return templateCategoryBits; }
        inline void AddTemplateCategoryBits(Physics::PhysicsLayer layer)
        {
            templateCategoryBits.SetBits(layer);
            RefreshColliders();
        }
        inline void RemoveTemplateCategoryBits(Physics::PhysicsLayer layer)
        {
            templateCategoryBits.ClearBits(layer);
            RefreshColliders();
        }
        inline void SetTemplateCategoryBits(Core::StateFlags<Physics::PhysicsLayer> flag)
        {
            templateCategoryBits.OverwriteFrom(flag);
            RefreshColliders();
        }

        inline Core::StateFlags<Physics::PhysicsLayer> GetTemplateMaskBits() const { return templateMaskBits; }
        inline void AddTemplateMaskBits(Physics::PhysicsLayer layer)
        {
            templateMaskBits.SetBits(layer);
            RefreshColliders();
        }
        inline void RemoveTemplateMaskBits(Physics::PhysicsLayer layer)
        {
            templateMaskBits.ClearBits(layer);
            RefreshColliders();
        }
        inline void SetTemplateMaskBits(Core::StateFlags<Physics::PhysicsLayer> flag)
        {
            templateMaskBits.OverwriteFrom(flag);
            RefreshColliders();
        }

        inline void SetTemplateGroupIndex(int16_t idx) { templateGroupIndex = idx; RefreshColliders(); }
        inline int16_t GetTemplateGroupIndex() const { return templateGroupIndex; }

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("id", static_cast<int>(ID));
            ctx.Write("layer", static_cast<uint32_t>(LayerMask.GetRawValue()));
            ctx.Write("renderQueue", static_cast<int>(RenderQueueType));
            ctx.Write("bodyType", static_cast<int>(bodyType));
            ctx.Write("fixedRotation", fixedRotation);
            ctx.Write("gravityScale", gravityScale);
            ctx.Write("linearDamping", linearDamping);
            ctx.Write("angularDamping", angularDamping);
            ctx.Write("awake", awake);
            ctx.Write("colliderOffset", colliderOffset);

            //==== 新增：序列化全部碰撞模板参数 ====
            ctx.Write("templateDensity", templateDensity);
            ctx.Write("templateFriction", templateFriction);
            ctx.Write("templateRestitution", templateRestitution);
            ctx.Write("templateIsTrigger", templateIsTrigger);
            ctx.Write("templateCategoryBits", templateCategoryBits.GetRawValue());
            ctx.Write("templateMaskBits", templateMaskBits.GetRawValue());
            ctx.Write("templateGroupIndex", static_cast<int>(templateGroupIndex));
        }

        void Deserialize(const Serialize::DeserializeContext& ctx) override
        {
            int id = static_cast<int>(ID);
            if (ctx.Read("id", id))
                ID = static_cast<Online::Config::TileMapID>(id);

            uint32_t layer = 0;
            ctx.Read("layer", layer);
            LayerMask = static_cast<Render::RenderLayer>(layer);

            int rq = static_cast<int>(RenderQueueType);
            ctx.Read("renderQueue", rq);
            RenderQueueType = static_cast<Render::RenderQueue>(rq);

            int bt = static_cast<int>(bodyType);
            if (ctx.Read("bodyType", bt))
                bodyType = static_cast<Online::Physics::BodyType>(bt);

            ctx.Read("fixedRotation", fixedRotation);
            ctx.Read("gravityScale", gravityScale);
            ctx.Read("linearDamping", linearDamping);
            ctx.Read("angularDamping", angularDamping);
            ctx.Read("awake", awake);

            ctx.Read("colliderOffset", colliderOffset);

            //==== 新增：反序列化读取模板配置 ====
            ctx.Read("templateDensity", templateDensity);
            ctx.Read("templateFriction", templateFriction);
            ctx.Read("templateRestitution", templateRestitution);
            ctx.Read("templateIsTrigger", templateIsTrigger);

            uint16_t catRaw{};
            if (ctx.Read("templateCategoryBits", catRaw))
            {
                Core::StateFlags<Physics::PhysicsLayer> tmp;
                tmp.OverwriteBits(static_cast<Physics::PhysicsLayer>(catRaw));
                SetTemplateCategoryBits(tmp);
            }
            uint16_t maskRaw{};
            if (ctx.Read("templateMaskBits", maskRaw))
            {
                Core::StateFlags<Physics::PhysicsLayer> tmp;
                tmp.OverwriteBits(static_cast<Physics::PhysicsLayer>(maskRaw));
                SetTemplateMaskBits(tmp);
            }
            int tgi{};
            if (ctx.Read("templateGroupIndex", tgi))
                templateGroupIndex = static_cast<int16_t>(tgi);

            RefreshColliders();
        }

        Asset::TextureID GetTexture()
        {
            return Asset::TileMapIDToTextureID(ID);
        }

        inline Core::StateFlags<Render::RenderLayer> GetLayerMask() const noexcept
        {
            return LayerMask;
        }

        inline Render::RenderQueue GetRenderQueue() const noexcept
        {
            return RenderQueueType;
        }

        inline Online::Config::TileMapID GetTileMapID() const
        {
            return ID;
        }

        inline void SetTileMapID(Online::Config::TileMapID newID)
        {
            if (ID != newID)
            {
                ID = newID;
                RefreshColliders();
            }
        }

        inline const SDL_Rect GetSize() const noexcept
        {
            glm::ivec2 size = Online::Asset::GetTextureSize(Asset::TileMapIDToTextureID(ID));
            return { 0, 0, size.x, size.y };
        }

        inline const SDL_FRect GetMapSize(glm::vec2 worldPos, glm::vec2 worldScale) const
        {
            return SDL_FRect{
                worldPos.x,
                worldPos.y,
                GetSize().w * worldScale.x,
                GetSize().h * worldScale.y
            };
        }

    private:
        Online::Config::TileMapID ID = Online::Config::TileMapID::Count;
        Core::StateFlags<Render::RenderLayer> LayerMask = Render::RenderLayer::Default;
        Render::RenderQueue RenderQueueType = Render::RenderQueue::World;

        Online::Physics::BodyType bodyType = Online::Physics::BodyType::Static;
        bool fixedRotation = true;
        float gravityScale = 1.0f;
        float linearDamping = 0.0f;
        float angularDamping = 0.0f;
        bool awake = true;

        glm::vec2 colliderOffset = glm::vec2(0.0f);

        std::vector<Collider> colliders;

        float templateDensity = 1.0f;
        float templateFriction = 0.3f;
        float templateRestitution = 0.0f;
        bool templateIsTrigger = false;
        Core::StateFlags<Physics::PhysicsLayer> templateCategoryBits = Physics::PhysicsLayer::Terrain;
        Core::StateFlags<Physics::PhysicsLayer> templateMaskBits = Core::StateFlags<Physics::PhysicsLayer>::Full();

        int16_t templateGroupIndex = 0;
    };
}