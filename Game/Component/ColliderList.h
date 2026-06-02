#pragma once
#include <Game/Component/Component.h>
#include <Game/Component/Collider.h>
#include <array>
#include <cstddef>
#include <Log/Common/FuncTable.h>

namespace Online::Game
{
    struct ColliderList : public Component
    {
    private:
        static constexpr size_t MAX_PER_OBJECT_COLLIDER = 8u;
    public:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("count", static_cast<uint32_t>(count));
            ctx.BeginArray("colliders");
            for (size_t i = 0; i < count; ++i)
            {
                Serialize::SerializeContext& elemCtx = ctx.WriteArrayObjectBegin();
                colliders[i].Serialize(elemCtx); 
            }
            ctx.EndArray();
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            count = 0;
            uint32_t readCount = 0;
            ctx.Read("count", readCount);

            size_t realLoad = (size_t)readCount;
            if (realLoad > MAX_PER_OBJECT_COLLIDER)
            {
                realLoad = MAX_PER_OBJECT_COLLIDER;
                Online::Log::Warning("ColliderList: 碰撞体数量超出上限，已截断");
            }

            const auto& arrayCtx = ctx.GetSubContext("colliders");
            size_t arrSize = 0;
            if (!arrayCtx.GetArraySize("", arrSize))
                return;

            realLoad = realLoad < arrSize ? realLoad : arrSize;
            for (size_t i = 0; i < realLoad; ++i)
            {
                const auto& elemCtx = arrayCtx.GetArrayElement(i);
                colliders[i].Deserialize(elemCtx);
            }
            count = realLoad;
        }

    public:
        Collider* AddCollider()
        {
            if (count >= MAX_PER_OBJECT_COLLIDER)
            {
                Online::Log::Warning("ColliderList: 单个物体碰撞体已达上限 " + std::to_string(MAX_PER_OBJECT_COLLIDER));
                return nullptr;
            }
            return &colliders[count++];
        }

        void RemoveColliderAt(size_t idx)
        {
            if (idx >= count) return;

            for (size_t i = idx; i < count - 1; ++i)
            {
                colliders[i] = colliders[i + 1];
            }
            count--;
        }

        void ClearColliders()
        {
            count = 0;
        }

        Collider* GetCollider(size_t idx)
        {
            if (idx >= count) return nullptr;
            return &colliders[idx];
        }

        size_t GetCount() const
        {
            return count;
        }

        std::array<Collider, MAX_PER_OBJECT_COLLIDER>& RawArray()
        {
            return colliders;
        }

    public:
        std::pair<Collider*, size_t> GetAllValid()
        {
            return { colliders.data(), count };
        }
        std::pair<const Collider*, size_t> GetAllValid() const
        {
            return { colliders.data(), count };
        }

        // 支持范围 for 遍历：for(auto& col : colliderList)
        Collider* begin() { return colliders.data(); }
        Collider* end() { return colliders.data() + count; }

        const Collider* begin() const { return colliders.data(); }
        const Collider* end()   const { return colliders.data() + count; }


    private:
        std::array<Collider, MAX_PER_OBJECT_COLLIDER> colliders{};
        size_t count = 0;
    };
}