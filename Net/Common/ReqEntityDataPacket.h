#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include <Serialize/Serializable.h>
#include <Net/Common/NetCommon.h>

namespace Online::Net
{
    struct ReqEntityDataPacket : public Online::Serialize::Serializable
    {
        static constexpr PacketType TYPE = PacketType::ReqEntityData;
        uint32_t targetNetId = 0;

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("targetNetId", targetNetId);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("targetNetId", targetNetId);
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