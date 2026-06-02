#pragma once
#include <cstdint>
#include <vector>
#include <span>

#include <SDL_net.h>

namespace Online::Net
{
    struct PacketHeader 
    {
        uint16_t type = 0;      // 应用层消息类型
        uint16_t version = 1;   // 协议版本
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

        void Reset()
        {
            id = -1;
            socket = nullptr;
            lastHeartbeatMs = 0;
            recvBuffer.clear();
            while (!sendQueue.empty()) sendQueue.pop();
            sendOffset = 0;
            inWriteSet = false;
        }
    };
}