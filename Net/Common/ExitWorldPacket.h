#pragma once
#include <Net/Common/NetCommon.h>
#include <Serialize/Serializable.h>

namespace Online::Net
{
    struct ExitWorldNotice : public Online::Serialize::Serializable
    {
        static constexpr PacketType TYPE = PacketType::ExitWorldNotice;
        uint32_t ClientID;

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("ClientID", ClientID);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("ClientID", ClientID);
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