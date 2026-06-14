#pragma once
#include <Net/Common/NetCommon.h>
#include <Serialize/Serializable.h>
#include <glm.hpp>

#include <Core/Utils/File.h>

namespace Online::Net
{
    struct EntityStateData
    {
        uint64_t netId;
        float x;
        float y;
        float rotation;
        float velocityX;
        float velocityY;
        uint32_t serverFrame;
    };

    struct EntityStatePacket : public Online::Serialize::Serializable
    {
        static constexpr PacketType TYPE = PacketType::EntityState;
        uint32_t entityCount = 0;
        std::vector<EntityStateData> entities;

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.BeginArray("entities");
            for (const auto& e : entities)
            {
                Online::Serialize::SerializeContext& elemCtx = ctx.WriteArrayObjectBegin();
                elemCtx.Write("netId", static_cast<uint32_t>(e.netId));
                elemCtx.Write("x", e.x);
                elemCtx.Write("y", e.y);
                elemCtx.Write("rotation", e.rotation);
                elemCtx.Write("velocityX", e.velocityX);
                elemCtx.Write("velocityY", e.velocityY);
                elemCtx.Write("serverFrame", e.serverFrame);
            }
            ctx.EndArray();
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            entities.clear();
            const auto& entitiesCtx = ctx.GetSubContext("entities");
            size_t count = 0;
            if (entitiesCtx.GetArraySize("", count))
            {
                for (size_t i = 0; i < count; ++i)
                {
                    const auto& elemCtx = entitiesCtx.GetArrayElement(i);
                    EntityStateData e;
                    uint32_t netId32 = 0;
                    elemCtx.Read("netId", netId32);
                    e.netId = netId32;
                    elemCtx.Read("x", e.x);
                    elemCtx.Read("y", e.y);
                    elemCtx.Read("rotation", e.rotation);
                    elemCtx.Read("velocityX", e.velocityX);
                    elemCtx.Read("velocityY", e.velocityY);
                    elemCtx.Read("serverFrame", e.serverFrame);
                    entities.push_back(e);
                }
            }
            entityCount = static_cast<uint32_t>(entities.size());
        }
        std::vector<std::byte> SerializePayload(Online::Serialize::API api = Online::Serialize::API::Json) const
        {
            std::vector<std::byte> out;
            SerializeToBytes(out, api);
            SerializeToFile(Core::GetExeDir() + "scenes\\sync.json");
            return out;
        }

        bool DeserializeFromPayload(std::span<const std::byte> payload, Online::Serialize::API api = Online::Serialize::API::Json)
        {
            return DeserializeFromBytes(payload, api);
        }
    };
}