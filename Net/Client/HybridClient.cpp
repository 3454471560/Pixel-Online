#include <Core/Time/time.h>
#include <Log/Common/FuncTable.h>
#include <Net/Client/HybridClient.h>

#include <cstring>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <queue>

bool Online::Net::Client::HybridClient::Initialize()
{
    if (enet_initialize() != 0) {
        throw std::runtime_error("ENet initialize failed");
    }

    clientHost = enet_host_create(nullptr, 1, 3, 0, 0);
    if (!clientHost) {
        enet_deinitialize();
        throw std::runtime_error("ENet client host create failed");
    }

    isRunning.store(true, std::memory_order_release);
    netThread = Online::Thread::RegisterThread("HybridClientThread", BootstrapNetThread, this, nullptr);
    if (netThread == Core::Thread::Identifier::Invalid) {
        enet_host_destroy(clientHost);
        enet_deinitialize();
        throw std::runtime_error("Failed to register client thread");
    }
    return true;
}

void Online::Net::Client::HybridClient::Release()
{
    isRunning.store(false, std::memory_order_release);
    Thread::UnregisterThread(netThread);

    auto allQueues = messageQueues.ExtractAll();
    for (auto& [key, q] : allQueues) delete q;
    messageQueues.Clear();

    if (clientHost) {
        enet_host_destroy(clientHost);
        clientHost = nullptr;
    }
    enet_deinitialize();
}

Online::Core::ThreadSafeQueue<Online::Net::NetMessage>&
Online::Net::Client::HybridClient::GetMessageQueue(PacketType type)
{
    uint16_t key = static_cast<uint16_t>(type);
    auto& queuePtr = messageQueues.GetOrCreate(key, []() -> Core::ThreadSafeQueue<Online::Net::NetMessage>*{
        return new Core::ThreadSafeQueue<Online::Net::NetMessage>();
        });
    return *queuePtr;
}

void Online::Net::Client::HybridClient::PushMessage(Online::Net::NetMessage&& msg)
{
    GetMessageQueue(static_cast<PacketType>(msg.header.type)).Push(std::move(msg));
}

bool Online::Net::Client::HybridClient::Connect(const std::string& host, uint16_t udpPort)
{
    {
        std::lock_guard lock(connMutex);
        if (connected) {
            ReleaseConnectionResources();
            connected = false;
        }
    }

    serverHost = host;
    serverPort = udpPort;

    ENetAddress address;
    if (enet_address_set_host(&address, host.c_str()) != 0) {
        throw std::runtime_error("ENet resolve host failed: " + host);
    }
    address.port = serverPort;

    ENetPeer* peer = enet_host_connect(clientHost, &address, 3, 0);
    if (!peer) {
        throw std::runtime_error("ENet connect failed");
    }

    bool handshakeDone = false;
    int assignedConnId = -1;
    const auto startTime = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(5);

    while (!handshakeDone && isRunning.load())
    {
        if (std::chrono::steady_clock::now() - startTime > timeout) {
            enet_peer_reset(peer);
            throw std::runtime_error("Connect timeout");
        }

        ENetEvent event;
        int ret = enet_host_service(clientHost, &event, 100);
        if (ret < 0) {
            enet_peer_reset(peer);
        }
        if (ret == 0) continue;

        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            break;

        case ENET_EVENT_TYPE_RECEIVE:
        {
            auto data = std::span<const std::byte>(
                reinterpret_cast<const std::byte*>(event.packet->data),
                event.packet->dataLength);
            if (data.size() < sizeof(PacketHeader)) {
                enet_packet_destroy(event.packet);
                break;
            }

            PacketHeader header;
            std::memcpy(&header, data.data(), sizeof(PacketHeader));

            if (header.type == static_cast<uint16_t>(PacketType::ConnID)) {
                if (header.length == sizeof(int)) {
                    assignedConnId = *reinterpret_cast<const int*>(data.data() + sizeof(PacketHeader));

                    PacketHeader handshakeHdr{};
                    handshakeHdr.type = static_cast<uint16_t>(PacketType::UdpHandshake);
                    handshakeHdr.length = sizeof(int);
                    std::vector<std::byte> handshake(sizeof(PacketHeader) + sizeof(int));
                    std::memcpy(handshake.data(), &handshakeHdr, sizeof(PacketHeader));
                    std::memcpy(handshake.data() + sizeof(PacketHeader), &assignedConnId, sizeof(int));

                    ENetPacket* pkt = enet_packet_create(handshake.data(), handshake.size(), ENET_PACKET_FLAG_RELIABLE);
                    if (pkt) {
                        enet_peer_send(peer, static_cast<uint8_t>(ChannelType::ReliableOrdered), pkt);
                        enet_host_flush(clientHost);
                        handshakeDone = true;
                    }
                    else {
                        throw std::runtime_error("Failed to create handshake packet");
                    }
                }
            }
            enet_packet_destroy(event.packet);
            break;
        }

        case ENET_EVENT_TYPE_DISCONNECT:
            enet_peer_reset(peer);
            throw std::runtime_error("Server rejected connection during handshake");

        default:
            break;
        }
    }

    if (!handshakeDone || assignedConnId == -1) {
        if (peer) enet_peer_reset(peer);
        throw std::runtime_error("Handshake failed");
    }

    {
        std::lock_guard lock(connMutex);
        serverPeer = peer;
        localConnId = assignedConnId;
        lastHeartbeatMs = Online::Core::CurrentMs();
        connected.store(true, std::memory_order_release);
    }

    Online::Log::Info("HybridClient connected, ID: " + std::to_string(localConnId));
    return true;
}

void Online::Net::Client::HybridClient::Disconnect()
{
    isRunning.store(false);
}

void Online::Net::Client::HybridClient::ReleaseConnectionResources()
{
    if (serverPeer) {
        enet_peer_disconnect(serverPeer, 0);
        enet_host_flush(clientHost);
        ENetEvent event;
        int waitLoops = 5;
        while (waitLoops-- > 0 && enet_host_service(clientHost, &event, 100) > 0) {
            if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                break;
            }
        }
        serverPeer = nullptr;
    }
    localConnId = -1;
}

bool Online::Net::Client::HybridClient::SendReliable(std::span<const std::byte> data, PacketType type, ChannelType channel)
{
    // 原子判断连接状态，不抢全局锁
    if (!connected.load(std::memory_order_acquire))
        return false;

    PendingPacket pkt;
    pkt.payload.assign(data.begin(), data.end());
    pkt.type = type;
    pkt.channel = channel;
    pkt.reliable = true;
    sendQueue.Push(std::move(pkt));
    return true;
}

bool Online::Net::Client::HybridClient::SendUnreliable(std::span<const std::byte> data, PacketType type, ChannelType channel)
{
    if (!connected.load(std::memory_order_acquire))
        return false;

    PendingPacket pkt;
    pkt.payload.assign(data.begin(), data.end());
    pkt.type = type;
    pkt.channel = channel;
    pkt.reliable = false;
    sendQueue.Push(std::move(pkt));
    return true;
}

bool Online::Net::Client::HybridClient::SendToServer(std::span<const std::byte> data,
    PacketType type,
    ChannelType channel,
    bool reliable)
{
    if (!serverPeer || !connected) return false;

    PacketHeader hdr{};
    hdr.type = static_cast<uint16_t>(type);
    hdr.length = static_cast<uint32_t>(data.size());

    std::vector<std::byte> packet(sizeof(PacketHeader) + data.size());
    std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
    if (!data.empty())
        std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());

    enet_uint32 flags = reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNSEQUENCED;
    ENetPacket* pkt = enet_packet_create(packet.data(), packet.size(), flags);
    if (!pkt) return false;

    return enet_peer_send(serverPeer, static_cast<enet_uint8>(channel), pkt) == 0;
}

bool Online::Net::Client::HybridClient::IsConnected() const
{
    return connected.load(std::memory_order_acquire);
}

int Online::Net::Client::HybridClient::GetLocalConnId() const
{
    return localConnId;
}

void Online::Net::Client::HybridClient::NetThread()
{
    while (isRunning.load(std::memory_order_acquire))
    {
        bool isConn = false;
        {
            std::lock_guard lock(connMutex);
            isConn = connected.load();

            if (isConn)
            {
                std::queue<PendingPacket> pendingQueue = sendQueue.PopAll();
                while (!pendingQueue.empty())
                {
                    auto pkt = std::move(pendingQueue.front());
                    pendingQueue.pop();
                    SendToServer(pkt.payload, pkt.type, pkt.channel, pkt.reliable);
                }

                ENetEvent event;
                while (enet_host_service(clientHost, &event, 0) > 0)
                {
                    switch (event.type)
                    {
                    case ENET_EVENT_TYPE_RECEIVE:
                        HandleReceive(event);
                        break;
                    case ENET_EVENT_TYPE_DISCONNECT:
                        Online::Log::Info("Server disconnected");
                        ReleaseConnectionResources();
                        connected = false;
                        isConn = false;
                        break;
                    default:
                        break;
                    }
                }

                uint32_t now = Online::Core::CurrentMs();
                if (now - lastHeartbeatMs > HEARTBEAT_INTERVAL)
                {
                    PacketHeader hdr{};
                    hdr.type = static_cast<uint16_t>(PacketType::Heartbeat);
                    hdr.length = 0;
                    std::vector<std::byte> beat(sizeof(PacketHeader));
                    std::memcpy(beat.data(), &hdr, sizeof(PacketHeader));

                    ENetPacket* pkt = enet_packet_create(beat.data(), beat.size(), ENET_PACKET_FLAG_UNSEQUENCED);
                    if (pkt)
                    {
                        enet_peer_send(serverPeer, static_cast<enet_uint8>(ChannelType::Unreliable), pkt);
                        lastHeartbeatMs = now;
                    }
                }
            }
        }

        if (!isConn)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    {
        std::lock_guard lock(connMutex);
        if (connected && serverPeer)
        {
            auto pending = sendQueue.PopAll();
            while (!pending.empty()) 
            {
                auto& pkt = pending.front();
                SendToServer(pkt.payload, pkt.type, pkt.channel, pkt.reliable);
                pending.pop();
            }

            enet_host_flush(clientHost);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            ReleaseConnectionResources();   
            connected = false;
        }
    }
}

void Online::Net::Client::HybridClient::HandleReceive(const ENetEvent& event)
{
    auto data = std::span<const std::byte>(
        reinterpret_cast<const std::byte*>(event.packet->data),
        event.packet->dataLength);
    if (data.size() < sizeof(PacketHeader)) {
        enet_packet_destroy(event.packet);
        return;
    }

    PacketHeader header;
    std::memcpy(&header, data.data(), sizeof(PacketHeader));

    if (header.type == static_cast<uint16_t>(PacketType::ConnID) ||
        header.type == static_cast<uint16_t>(PacketType::UdpHandshake) ||
        header.type == static_cast<uint16_t>(PacketType::Heartbeat))
    {
        if (header.type == static_cast<uint16_t>(PacketType::Heartbeat))
            lastHeartbeatMs = Online::Core::CurrentMs();
        enet_packet_destroy(event.packet);
        return;
    }

    Online::Net::NetMessage msg;
    msg.connectionId = localConnId;
    msg.header = header;
    msg.body.assign(data.begin() + sizeof(PacketHeader), data.end());
    PushMessage(std::move(msg));
    enet_packet_destroy(event.packet);
}

void Online::Net::Client::HybridClient::Tick()
{
    static const uint32_t TIMEOUT = 30000;
    uint32_t now = Online::Core::CurrentMs();
    if (connected.load(std::memory_order_acquire) && (now - lastHeartbeatMs > TIMEOUT)) {
        Online::Log::Info("Heartbeat timeout, disconnecting");
        std::lock_guard lock(connMutex);
        ReleaseConnectionResources();
        connected = false;
    }
}