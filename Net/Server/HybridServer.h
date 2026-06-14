#pragma once
#include <Core/Allocate/Allocate.h>
#include <Core/Thread/Thread.h>
#include <Core/ThreadSafe/ThreadSafeHashMap.h>
#include <Core/ThreadSafe/ThreadSafeQueue.h>
#include <Core/ObjectPool/ObjectPool.h>
#include <Log/Common/LogLevel.h>
#include <Context/Common/Module.h>
#include <Thread/Common/FuncTable.h>
#include <Net/Common/NetCommon.h>

#include <enet/enet.h>

#include <atomic>
#include <unordered_map>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>

namespace Online::Net::Server
{
    class HybridServer
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<HybridServer>;
        private:
            static HybridServer* Create() { return ONLINE_NEW(HybridServer); }
            static void Destroy(HybridServer* s) { ONLINE_DELETE(s); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<HybridServer>;
        private:
            static bool Initialize(HybridServer* s) { return s->Initialize(); }
            static void Release(HybridServer* s) { s->Release(); }
            static void LateUpdate(HybridServer* s) { s->Tick(); }
        };

    private:
        HybridServer() = default;
        ~HybridServer() = default;

    public:
        HybridServer(const HybridServer&) = delete;
        HybridServer& operator=(const HybridServer&) = delete;
        HybridServer(HybridServer&&) = delete;
        HybridServer& operator=(HybridServer&&) = delete;

    private:
        bool Initialize();
        void Release();
        void Tick();

        inline static void BootstrapNetThread(void* log, void*) {
            static_cast<HybridServer*>(log)->NetThreadFunc();
        }

        void NetThreadFunc();

        void AcceptConnection(ENetPeer* peer);
        void ReleaseConnectionResources(Connection* conn);

        void HandleConnect(const ENetEvent& event);
        void HandleReceive(const ENetEvent& event);
        void HandleDisconnect(const ENetEvent& event);

        bool SendClientId(ENetPeer* peer, int connId);
        void PushMessage(NetMessage&& msg);

    public:
        Core::ThreadSafeQueue<NetMessage>& GetMessageQueue(PacketType type);

        bool SendReliable(int connectionId, std::span<const std::byte> data,
            PacketType type,
            ChannelType channel = ChannelType::ReliableOrdered);
        bool BroadcastReliable(std::span<const std::byte> data, PacketType type,
            ChannelType channel = ChannelType::ReliableOrdered);

        bool SendUnreliable(int connectionId, std::span<const std::byte> data,
            PacketType type,
            ChannelType channel = ChannelType::Unreliable);
        bool BroadcastUnreliable(std::span<const std::byte> data,
            PacketType type,
            ChannelType channel = ChannelType::Unreliable);
        bool BroadcastUnreliableExcept(int excludeConnId,
            std::span<const std::byte> data,
            PacketType type,
            ChannelType channel = ChannelType::Unreliable);

        void CloseConnection(int connectionId);
        int GetConnectionCount() const;

    private:
        uint16_t port = 7778;

        ENetHost* host = nullptr;

        mutable std::mutex connMutex;
        Core::ObjectPool<Online::Net::Connection> connPool{
            [](Online::Net::Connection* c) { c->Reset(); },
            [](Online::Net::Connection* c) { c->Reset(); },
            64
        };
        std::unordered_map<int, Connection*> connections;
        std::unordered_map<ENetPeer*, int> peerToConnId;

        int nextConnectionId = 1;

        Core::ThreadSafeHashMap<uint16_t, Core::ThreadSafeQueue<NetMessage>*> messageQueues;

        std::atomic<bool> running = false;
        Core::Thread::Identifier netThreadId;
    };
}