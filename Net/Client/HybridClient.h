#pragma once
#include <Core/Allocate/Allocate.h>
#include <Core/Thread/Thread.h>
#include <Core/ThreadSafe/ThreadSafeHashMap.h>
#include <Core/ThreadSafe/ThreadSafeQueue.h>
#include <Log/Common/LogLevel.h>
#include <Context/Common/Module.h>
#include <Thread/Common/FuncTable.h>
#include <Net/Common/NetCommon.h>

#include <enet/enet.h>

#include <atomic>
#include <memory>
#include <vector>
#include <string>
#include <mutex>

namespace Online::Net::Client
{
    class HybridClient
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<HybridClient>;
        private:
            static HybridClient* Create() { return ONLINE_NEW(HybridClient); }
            static void Destroy(HybridClient* c) { ONLINE_DELETE(c); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<HybridClient>;
        private:
            static bool Initialize(HybridClient* c) { return c->Initialize(); }
            static void Release(HybridClient* c) { c->Release(); }
            static void LateUpdate(HybridClient* c) { c->Tick(); }
        };

    private:
        HybridClient() = default;
        ~HybridClient() = default;

    public:
        HybridClient(const HybridClient&) = delete;
        HybridClient& operator=(const HybridClient&) = delete;
        HybridClient(HybridClient&&) = delete;
        HybridClient& operator=(HybridClient&&) = delete;

    public:
        bool Connect(const std::string& host, uint16_t udpPort);
        void Disconnect();

        bool SendReliable(std::span<const std::byte> data, PacketType type,
            ChannelType channel = ChannelType::ReliableOrdered);
        bool SendUnreliable(std::span<const std::byte> data,
            PacketType type,
            ChannelType channel = ChannelType::Unreliable);

        Core::ThreadSafeQueue<Online::Net::NetMessage>& GetMessageQueue(PacketType type);

        bool IsConnected() const;
        int GetLocalConnId() const;

    private:
        bool Initialize();
        void Release();
        void Tick();

        inline static void BootstrapNetThread(void* log, void*)
        {
            static_cast<HybridClient*>(log)->NetThread();
        }
        void NetThread();

        bool SendToServer(std::span<const std::byte> data, PacketType type,
            ChannelType channel, bool reliable);

        void HandleReceive(const ENetEvent& event);

        void PushMessage(Online::Net::NetMessage&& msg);

        void ReleaseConnectionResources();

    private:
        std::atomic<bool> isRunning = false;
        Core::Thread::Identifier netThread;

        std::string serverHost;
        uint16_t serverPort = 7778;

        ENetHost* clientHost = nullptr;
        ENetPeer* serverPeer = nullptr;

        std::mutex connMutex;
        std::atomic<bool> connected = false;

        int localConnId = -1;
        uint32_t lastHeartbeatMs = 0;

        Core::ThreadSafeHashMap<uint16_t, Core::ThreadSafeQueue<Online::Net::NetMessage>*> messageQueues;

        static constexpr uint32_t HEARTBEAT_INTERVAL = 5000;

        struct PendingPacket
        {
            std::vector<std::byte> payload;
            PacketType type;
            ChannelType channel;
            bool reliable;
        };

        Core::ThreadSafeQueue<PendingPacket> m_sendQueue;
    };
}