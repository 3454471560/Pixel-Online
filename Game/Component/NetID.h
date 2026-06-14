#pragma once
#include <Game/Component/Component.h>
#include <cstdint>

namespace Online::Game
{
    struct NetID : public Component
    {
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("netId", netId);
            ctx.Write("ownerConnId", ownerConnId);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("netId", netId);
            ctx.Read("ownerConnId", ownerConnId);
        }

    public:
        inline uint32_t GetNetId()
        {
            return netId;
        }
        inline uint32_t GetOwnerConnId()
        {
            return ownerConnId;
        }
        inline bool GetNeedSync()
        {
            return needSync;
        }

        inline void SetNetId(uint32_t id)
        {
            netId = id;
        }
        inline void SetOwnerConnId(uint32_t id)
        {
            ownerConnId = id;
        }
        inline void SetNeedSync(bool flag)
        {
            needSync = flag;
        }

    private:
        uint32_t netId = 0;
        uint32_t ownerConnId = 0;
        bool needSync = true;
    };
}