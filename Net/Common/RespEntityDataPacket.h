#pragma once
#include <vector>
#include <span>
#include <Serialize/Serializable.h>
#include <Net/Common/NetCommon.h>
#include <Net/Common/EntityFullData.h>

namespace Online::Net
{
    struct RespEntityDataPacket : public Online::Serialize::Serializable
    {
        static constexpr PacketType TYPE = PacketType::RespEntityData;
        EntityFullData entityData;

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            entityData.Serialize(ctx.GetSubContext("entityData"));
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            entityData.Deserialize(ctx.GetSubContext("entityData"));
        }

        std::vector<std::byte> SerializePayload() const
        {
            std::vector<std::byte> out;
            SerializeToBytes(out);
            return out;
        }

        bool DeserializeFromPayload(std::span<const std::byte> payload)
        {
            return DeserializeFromBytes(payload);
        }
    };
}