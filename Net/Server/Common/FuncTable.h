#pragma once
#include<Context/Context.h>
#include<Server/Context/ServerContext.h>
#include<Core/ThreadSafe/ThreadSafeQueue.h>
#include<Net/Common/NetCommon.h>
#include<stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Net::Server::HybridServer>
    {
        friend class Online::Runtime::Server;
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
            if (!OnGetMessageQueue) { throw std::runtime_error("Delegates miss [OnGetMessageQueue] Function!"); }
            if (!OnSendReliable) { throw std::runtime_error("Delegates miss [OnSendReliable] Function!"); }
            if (!OnSendUnreliable) { throw std::runtime_error("Delegates miss [OnSendUnreliable] Function!"); }
            if (!OnBroadcastReliable) { throw std::runtime_error("Delegates miss [OnBroadcastReliable] Function!"); }
            if (!OnBroadcastUnreliable) { throw std::runtime_error("Delegates miss [OnBroadcastUnreliable] Function!"); }
            return true;
        }

        inline void UnRegister() noexcept
        {
            OnGetMessageQueue = nullptr;
            OnSendReliable = nullptr;
            OnSendUnreliable = nullptr;
            OnBroadcastReliable = nullptr;
            OnBroadcastUnreliable = nullptr;
        }

    public:
        inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>&
            InvokeOnGetMessageQueue(Online::Net::PacketType type) noexcept
        {
            return OnGetMessageQueue(type);
        }

        inline bool InvokeOnSendReliable(int connectionId,
            std::span<const std::byte> data,
            Online::Net::PacketType type,
            Online::Net::ChannelType channel) noexcept
        {
            return OnSendReliable(connectionId, data, type, channel);
        }

        inline bool InvokeOnSendUnreliable(int connectionId,
            std::span<const std::byte> data,
            Online::Net::PacketType type,
            Online::Net::ChannelType channel) noexcept
        {
            return OnSendUnreliable(connectionId, data, type, channel);
        }

        inline bool InvokeOnBroadcastReliable(std::span<const std::byte> data,
            Online::Net::PacketType type,
            Online::Net::ChannelType channel) noexcept
        {
            return OnBroadcastReliable(data, type, channel);
        }

        inline bool InvokeOnBroadcastUnreliable(std::span<const std::byte> data,
            Online::Net::PacketType type,
            Online::Net::ChannelType channel) noexcept
        {
            return OnBroadcastUnreliable(data, type, channel);
        }

    private:
        Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& (*OnGetMessageQueue)(Online::Net::PacketType) noexcept = nullptr;
        bool (*OnSendReliable)(int, std::span<const std::byte>, Online::Net::PacketType, Online::Net::ChannelType) noexcept = nullptr;
        bool (*OnSendUnreliable)(int, std::span<const std::byte>, Online::Net::PacketType, Online::Net::ChannelType) noexcept = nullptr;
        bool (*OnBroadcastReliable)(std::span<const std::byte>, Online::Net::PacketType, Online::Net::ChannelType) noexcept = nullptr;
        bool (*OnBroadcastUnreliable)(std::span<const std::byte>, Online::Net::PacketType, Online::Net::ChannelType) noexcept = nullptr;
    };
}

namespace Online::Net::Server
{
    inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& GetMessageQueue(PacketType type) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerFuncTable<Online::Net::Server::HybridServer>()
            .InvokeOnGetMessageQueue(type);
    }

    inline bool SendReliable(int connectionId,
        std::span<const std::byte> data,
        PacketType type,
        ChannelType channel = ChannelType::ReliableOrdered) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerFuncTable<Online::Net::Server::HybridServer>()
            .InvokeOnSendReliable(connectionId, data, type, channel);
    }

    inline bool SendUnreliable(int connectionId,
        std::span<const std::byte> data,
        PacketType type,
        ChannelType channel = ChannelType::Unreliable) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerFuncTable<Online::Net::Server::HybridServer>()
            .InvokeOnSendUnreliable(connectionId, data, type, channel);
    }

    inline bool BroadcastReliable(std::span<const std::byte> data,
        PacketType type,
        ChannelType channel = ChannelType::ReliableOrdered) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerFuncTable<Online::Net::Server::HybridServer>()
            .InvokeOnBroadcastReliable(data, type, channel);
    }

    inline bool BroadcastUnreliable(std::span<const std::byte> data,
        PacketType type,
        ChannelType channel = ChannelType::Unreliable) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerFuncTable<Online::Net::Server::HybridServer>()
            .InvokeOnBroadcastUnreliable(data, type, channel);
    }
}