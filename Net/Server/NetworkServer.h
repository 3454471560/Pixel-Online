#pragma once
#include <Core/Allocate/Allocate.h>
#include <Core/Thread/Thread.h>
#include <Core/ThreadSafe/ThreadSafeQueue.h>
#include <Core/ObjectPool/ObjectPool.h>
#include <Log/Common/LogLevel.h>
#include <Context/Common/Module.h>
#include <Thread/Common/FuncTable.h>
#include <Net/Common/NetCommon.h>

#include <SDL_net.h>

#include <atomic>
#include <unordered_map>
#include <memory>
#include <vector>
#include <queue>

namespace Online::Net::Server
{
    class NetworkServer
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<NetworkServer>;
        private:
            static NetworkServer* Create()
            {
                return ONLINE_NEW(NetworkServer);
            }
            static void Destroy(NetworkServer* s)
            {
                ONLINE_DELETE(s);
            }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<NetworkServer>;
        private:
            static bool Initialize(NetworkServer* s) { return s->Initialize(); }
            static void Release(NetworkServer* s) { s->Release(); }
            static void LateUpdate(NetworkServer* s) { s->Tick(); }
        };

    private:
        NetworkServer() = default;
        ~NetworkServer() = default;

    public:
        NetworkServer(const NetworkServer&) = delete;
        NetworkServer& operator=(const NetworkServer&) = delete;
        NetworkServer(NetworkServer&&) = delete;
        NetworkServer& operator=(NetworkServer&&) = delete;

    private:
        bool Initialize();
        void Release();

    private:
        inline static void BootstrapNetThread(void* log, void*)
        {
            static_cast<Online::Net::Server::NetworkServer*>(log)->NetThread();
        }

        void Tick();
        inline void SetListenPort(uint16_t p) { port = p; }

        void NetThread();
        void AcceptNewConnection();
        void ReleaseConnectionResources(Online::Net::Connection* conn);
        bool SendLocked(int connectionId, std::span<const std::byte> data);

        bool TryReceiveNonBlocking(Connection* conn);
        bool ParseMessages(Connection* conn); 

    public:
        inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& GetMessageQueue() { return messages; }
        bool Send(int connectionId, std::span<const std::byte> data);
        bool Broadcast(std::span<const std::byte> data);
        void CloseConnection(int connectionId);

    private:
        uint16_t port = 7777;

        SDLNet_SocketSet readSet = nullptr;
        SDLNet_SocketSet writeSet = nullptr;
        TCPsocket serverSocket = nullptr;

        std::mutex connMutex;
        Core::ObjectPool<Online::Net::Connection> connPool{
            [](Online::Net::Connection* c) { c->Reset(); },
            [](Online::Net::Connection* c) { c->Reset(); },
            64
        };
        std::unordered_map<int, Online::Net::Connection*> connections;
        int nextConnectionId = 1;

        Online::Core::ThreadSafeQueue<Online::Net::NetMessage> messages;
    };
}