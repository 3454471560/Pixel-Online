#pragma once
#include <Net/Common/NetCommon.h>
#include <Serialize/Serializable.h>

namespace Online::Net
{
    struct PlayerInputPacket : public Online::Serialize::Serializable
    {
        // 固定包类型标识（与 NetCommon.h 枚举对应）
        static constexpr PacketType TYPE = PacketType::PlayerInput;

        // 身份标识：与项目现有实体/连接ID类型对齐
        uint32_t netId = 0;        // 玩家实体网络ID
        int connId = -1;           // 客户端连接ID（沿用 EntityFullData 的 int 类型）

        // 游戏业务按键状态（最小输入子集，不上传全量键盘）
        bool keyA_Hold = false;    // A键 持续按住（左移）
        bool keyD_Hold = false;    // D键 持续按住（右移）
        bool keySpace_Press = false;// 空格 瞬时按下（跳跃）

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("netId", netId);
            ctx.Write("connId", connId);
            ctx.Write("keyA_Hold", keyA_Hold);
            ctx.Write("keyD_Hold", keyD_Hold);
            ctx.Write("keySpace_Press", keySpace_Press);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("netId", netId);
            ctx.Read("connId", connId);
            ctx.Read("keyA_Hold", keyA_Hold);
            ctx.Read("keyD_Hold", keyD_Hold);
            ctx.Read("keySpace_Press", keySpace_Press);
        }

        std::vector<std::byte> SerializePayload(Online::Serialize::API api = Online::Serialize::API::Json) const
        {
            std::vector<std::byte> out;
            SerializeToBytes(out, api);
            return out;
        }

        bool DeserializeFromPayload(std::span<const std::byte> payload, Online::Serialize::API api = Online::Serialize::API::Json)
        {
            return DeserializeFromBytes(payload, api);
        }
    };
}