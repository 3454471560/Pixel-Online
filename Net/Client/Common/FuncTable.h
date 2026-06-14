#pragma once
#include<Context/Context.h>
#include<Client/Context/ClientContext.h>
#include<Core/ThreadSafe/ThreadSafeQueue.h>
#include<Net/Common/NetCommon.h>
#include<stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Net::Client::HybridClient>
    {
        friend class Online::Runtime::Client;
    private:
        FuncTable() = default;
        ~FuncTable() = default;

    public:
        FuncTable(const FuncTable&) = delete;
        FuncTable& operator=(const FuncTable&) = delete;
        FuncTable(FuncTable&&) = delete;
        FuncTable& operator=(FuncTable&&) = delete;

    public:
        inline bool Check() const
        {
            if (!OnConnect) { throw std::runtime_error("Delegates miss [OnConnect] Function!"); }
            if (!OnDisconnect) { throw std::runtime_error("Delegates miss [OnDisconnect] Function!"); }
            if (!OnGetMessageQueue) { throw std::runtime_error("Delegates miss [OnGetMessageQueue] Function!"); }
            if (!OnSendReliable) { throw std::runtime_error("Delegates miss [OnSendReliable] Function!"); }
            if (!OnSendUnreliable) { throw std::runtime_error("Delegates miss [OnSendUnreliable] Function!"); }
            if (!OnIsConnected) { throw std::runtime_error("Delegates miss [OnIsConnected] Function!"); }
            if (!OnGetLocalConnId) { throw std::runtime_error("Delegates miss [OnGetLocalConnId] Function!"); }
            return true;
        }

        inline void UnRegister() noexcept
        {
            OnConnect = nullptr;
            OnDisconnect = nullptr;
            OnGetMessageQueue = nullptr;
            OnSendReliable = nullptr;
            OnSendUnreliable = nullptr;
            OnIsConnected = nullptr;
            OnGetLocalConnId = nullptr;
        }

    public:
        inline bool InvokeOnConnect(const std::string& host, uint16_t udpPort) noexcept
        {
            return OnConnect(host, udpPort);
        }

        inline void InvokeOnDisconnect() noexcept
        {
            OnDisconnect();
        }

        inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>&
            InvokeOnGetMessageQueue(Online::Net::PacketType type) noexcept
        {
            return OnGetMessageQueue(type);
        }

        inline bool InvokeOnSendReliable(std::span<const std::byte> data,
            Online::Net::PacketType type,
            Online::Net::ChannelType channel) noexcept
        {
            return OnSendReliable(data, type, channel);
        }

        inline bool InvokeOnSendUnreliable(std::span<const std::byte> data,
            Online::Net::PacketType type,
            Online::Net::ChannelType channel) noexcept
        {
            return OnSendUnreliable(data, type, channel);
        }

        inline bool InvokeOnIsConnected() noexcept
        {
            return OnIsConnected();
        }

        inline int InvokeGetLocalConnId() noexcept
        {
            return OnGetLocalConnId();
        }

    private:
        bool (*OnConnect)(const std::string&, uint16_t) noexcept = nullptr;
        void (*OnDisconnect)() noexcept = nullptr;
        Online::Core::ThreadSafeQueue<Online::Net::NetMessage>&
            (*OnGetMessageQueue)(Online::Net::PacketType) noexcept = nullptr;
        bool (*OnSendReliable)(std::span<const std::byte>, Online::Net::PacketType, Online::Net::ChannelType) noexcept = nullptr;
        bool (*OnSendUnreliable)(std::span<const std::byte>, Online::Net::PacketType, Online::Net::ChannelType) noexcept = nullptr;
        bool (*OnIsConnected)() noexcept = nullptr;
        int (*OnGetLocalConnId)() noexcept = nullptr;
    };
}

namespace Online::Net::Client
{
    // 便利函数：连接
    inline bool Connect(const std::string& host, uint16_t udpPort) noexcept
    {
        return Online::Runtime::ClientContext::Instance()
            .GetClientFuncTable<Online::Net::Client::HybridClient>()
            .InvokeOnConnect(host, udpPort);
    }

    // 便利函数：断开
    inline void Disconnect() noexcept
    {
        Online::Runtime::ClientContext::Instance()
            .GetClientFuncTable<Online::Net::Client::HybridClient>()
            .InvokeOnDisconnect();
    }

    // 便利函数：获取消息队列
    inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>&
        GetMessageQueue(Online::Net::PacketType type) noexcept
    {
        return Online::Runtime::ClientContext::Instance()
            .GetClientFuncTable<Online::Net::Client::HybridClient>()
            .InvokeOnGetMessageQueue(type);
    }

    // 便利函数：可靠发送（默认通道为可靠有序）
    inline bool SendReliable(std::span<const std::byte> data,
        Online::Net::PacketType type,
        Online::Net::ChannelType channel = Online::Net::ChannelType::ReliableOrdered) noexcept
    {
        return Online::Runtime::ClientContext::Instance()
            .GetClientFuncTable<Online::Net::Client::HybridClient>()
            .InvokeOnSendReliable(data, type, channel);
    }

    // 便利函数：不可靠发送（默认通道为不可靠）
    inline bool SendUnreliable(std::span<const std::byte> data,
        Online::Net::PacketType type,
        Online::Net::ChannelType channel = Online::Net::ChannelType::Unreliable) noexcept
    {
        return Online::Runtime::ClientContext::Instance()
            .GetClientFuncTable<Online::Net::Client::HybridClient>()
            .InvokeOnSendUnreliable(data, type, channel);
    }

    // 便利函数：查询连接状态
    inline bool IsConnected() noexcept
    {
        return Online::Runtime::ClientContext::Instance()
            .GetClientFuncTable<Online::Net::Client::HybridClient>()
            .InvokeOnIsConnected();
    }

    // 便利函数：获取本地连接 ID
    inline int GetLocalConnId() noexcept
    {
        return Online::Runtime::ClientContext::Instance()
            .GetClientFuncTable<Online::Net::Client::HybridClient>()
            .InvokeGetLocalConnId();
    }
}