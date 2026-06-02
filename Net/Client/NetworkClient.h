#pragma once
#include <Core/Allocate/Allocate.h>
#include <Core/Thread/Thread.h>
#include <Core/ThreadSafe/ThreadSafeQueue.h>
#include <Log/Common/LogLevel.h>
#include <Context/Common/Module.h>
#include <Thread/Common/FuncTable.h>
#include <Net/Common/NetCommon.h>

#include <SDL_net.h>

#include <atomic>
#include <memory>
#include <vector>
#include <queue>
#include <string>

namespace Online::Net::Client
{
    class NetworkClient
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<NetworkClient>;
        private:
            static NetworkClient* Create()
            {
                return ONLINE_NEW(NetworkClient);
            }
            static void Destroy(NetworkClient* c)
            {
                ONLINE_DELETE(c);
            }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<NetworkClient>;
        private:
            static bool Initialize(NetworkClient* c) { return c->Initialize(); }
            static void Release(NetworkClient* c) { c->Release(); }
            static void LateUpdate(NetworkClient* c) { c->Tick(); }
        };

    private:
        NetworkClient() = default;
        ~NetworkClient() = default;

    public:
        NetworkClient(const NetworkClient&) = delete;
        NetworkClient& operator=(const NetworkClient&) = delete;
        NetworkClient(NetworkClient&&) = delete;
        NetworkClient& operator=(NetworkClient&&) = delete;

    public:
        bool Connect(const std::string& host, uint16_t port);
        void Disconnect();
        bool Send(std::span<const std::byte> data);
        inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& GetMessageQueue() { return messages; }
        bool IsConnected() const;
    private:
        bool Initialize();
        void Release();
        void Tick();

        inline static void BootstrapNetThread(void* log, void*)
        {
            static_cast<Online::Net::Client::NetworkClient*>(log)->NetThread();
        }
        void NetThread();

        void ReleaseConnectionResources();
        void CloseConnection();

        bool TryReceiveNonBlocking();
        bool ParseMessages();
        bool SendLocked(std::span<const std::byte> data);
        bool RecvAll(TCPsocket sock, void* buffer, int len);
    private:
        std::atomic<bool> isRunning = false;
        Online::Core::Thread::Identifier netThread;

        std::string serverHost;
        uint16_t serverPort = 7777;

        SDLNet_SocketSet readSet = nullptr;
        SDLNet_SocketSet writeSet = nullptr;

        std::mutex connMutex;
        Online::Net::Connection connection;
        std::atomic<bool> connected = false;

        Online::Core::ThreadSafeQueue<Online::Net::NetMessage> messages;
    };
}