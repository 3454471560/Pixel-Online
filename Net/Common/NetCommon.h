#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include <queue>

#include <enet/enet.h>

namespace Online::Net
{
    enum class PacketType : uint16_t
    {
        ConnID = 0,   // 服务器向客户端分配连接 ID（建立连接时使用）
        UdpHandshake = 1,   // UDP 握手（绑定 TCP 连接 ID）
        Heartbeat = 2,   // UDP 心跳

        EntityState = 10,  // 实体状态同步
        JoinWorldRequest = 11,  // 客户端→服务端：申请加入世界
		WorldSnapshot = 12, // 服务端→客户端：世界快照
        ReqEntityData,    // 客户端：请求指定NetID实体数据
        RespEntityData,  // 服务端：返回指定NetID实体完整数据
        PlayerInput = 15,   // 客户端 → 服务端：玩家按键输入包
    };

    enum class ChannelType : uint8_t
    {
        ReliableOrdered = 0,  // 可靠有序（聊天、RPC）
        ReliableUnordered = 1,  // 可靠无序（技能释放等）
        Unreliable = 2,  // 不可靠（位置同步）
    };

    struct PacketHeader
    {
        uint16_t type = 0;      // 应用层消息类型
        uint32_t length = 0;    // 后续 payload 长度（字节）
        uint32_t seq = 0;       // 可选：序列号
    };

    struct NetMessage
    {
        int connectionId = -1;
        PacketHeader header{};
        std::vector<std::byte> body;
    };

    inline std::span<const std::byte> HeaderAsBytes(const PacketHeader& h) noexcept {
        return std::span<const std::byte>(reinterpret_cast<const std::byte*>(&h), sizeof(PacketHeader));
    }
    inline std::span<std::byte> HeaderAsBytes(PacketHeader& h) noexcept {
        return std::span<std::byte>(reinterpret_cast<std::byte*>(&h), sizeof(PacketHeader));
    }

    struct Connection 
    {
        int id = -1;
        ENetPeer* peer = nullptr;

        uint32_t lastHeartbeatMs = 0;

        std::vector<std::byte> recvBuffer;
        bool handshakeCompleted = false;
      
        void Reset() 
        {
            id = -1;
            peer = nullptr;
            lastHeartbeatMs = 0;
            handshakeCompleted = false;
        }
    };
}