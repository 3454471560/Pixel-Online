#pragma once
#include <Net/Common/NetCommon.h>
#include <Serialize/Serializable.h>
#include <Game/Component/NetID.h>

namespace Online::Net
{
    struct JoinWorldRequest : public Online::Serialize::Serializable
    {
        static constexpr PacketType TYPE = PacketType::JoinWorldRequest;
        std::string playerName;
        uint32_t playerId;

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("playerName", playerName);
            ctx.Write("playerId", playerId);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("playerName", playerName);
            ctx.Read("playerId", playerId);
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