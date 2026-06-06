#pragma once
#include <cstdint>
#include <vector>
#include <span>

#include <SDL_net.h>

namespace Online::Net
{
    enum class PacketType : uint16_t
    {
        Invalid = 0,
        Heartbeat = 1,        // 心跳包（无Body）
        ConnectionId = 2,     // 连接ID分配包
        NormalMessage = 3,    // 普通业务消息
        Disconnect = 4        // 主动断开连接
    };

    enum class NetErrorCode : int
    {
        Success = 0,
        SDLNetInitFailed = 1,
        ResolveHostFailed = 2,
        TcpOpenFailed = 3,
        AddSocketFailed = 4,
        RecvFailed = 5,
        SendFailed = 6,
        HeartbeatTimeout = 7,
        OversizedPacket = 8,
        InvalidPacketLength = 9,
        ConnectionClosed = 10
    };

    inline std::string NetErrorCodeToString(NetErrorCode code) noexcept
    {
        switch (code)
        {
        case NetErrorCode::Success: return "Success";
        case NetErrorCode::SDLNetInitFailed: return "SDLNet init failed";
        case NetErrorCode::ResolveHostFailed: return "Resolve host failed";
        case NetErrorCode::TcpOpenFailed: return "TCP open failed";
        case NetErrorCode::AddSocketFailed: return "Add socket to set failed";
        case NetErrorCode::RecvFailed: return "Receive data failed";
        case NetErrorCode::SendFailed: return "Send data failed";
        case NetErrorCode::HeartbeatTimeout: return "Heartbeat timeout";
        case NetErrorCode::OversizedPacket: return "Oversized packet received";
        case NetErrorCode::InvalidPacketLength: return "Invalid packet length";
        case NetErrorCode::ConnectionClosed: return "Connection closed by peer";
        default: return "Unknown error (" + std::to_string(static_cast<int>(code)) + ")";
        }
    }

    inline uint32_t CalculateCRC32(const std::span<const std::byte> data) noexcept
    {
        uint32_t crc = 0xFFFFFFFF;
        for (const std::byte b : data)
        {
            crc ^= static_cast<uint8_t>(b);
            for (int i = 0; i < 8; ++i)
            {
                crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
            }
        }
        return ~crc;
    }

    struct PacketHeader 
    {
        uint16_t type = 0;      // 应用层消息类型
        uint32_t length = 0;    // 后续 payload 长度（字节）
        uint32_t seq = 0;       // 可选：序列号，用于重传/丢包统计
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
        TCPsocket socket = nullptr;
        uint32_t lastHeartbeatMs = 0;
        std::vector<std::byte> recvBuffer;

        std::queue<std::vector<std::byte>> sendQueue;
        size_t sendOffset = 0;
        bool inWriteSet = false;

        uint32_t nextSendSeq = 1;

        void Reset()
        {
            id = -1;
            socket = nullptr;
            lastHeartbeatMs = 0;
            recvBuffer.clear();
            while (!sendQueue.empty()) sendQueue.pop();
            sendOffset = 0;
            inWriteSet = false;
            nextSendSeq = 1;
        }
    };

}
