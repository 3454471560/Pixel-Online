#include <Core/Time/time.h>
#include <Log/Common/FuncTable.h>
#include <Net/Server/HybridServer.h>
#include <cstring>
#include <stdexcept>
#include <thread>

bool Online::Net::Server::HybridServer::Initialize()
{
    if (enet_initialize() != 0)
        throw std::runtime_error("ENet initialize failed");

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    // 创建服务端 host：最大 64 个客户端，3 个通道
    host = enet_host_create(&address, 64, 3, 0, 0);
    if (!host) {
        enet_deinitialize();
        throw std::runtime_error("ENet host create failed");
    }

    running.store(true, std::memory_order_release);
    netThreadId = Online::Thread::RegisterThread("HybridNetThread", BootstrapNetThread, this, nullptr);
    if (netThreadId == Core::Thread::Identifier::Invalid) {
        enet_host_destroy(host);
        enet_deinitialize();
        throw std::runtime_error("Failed to register net thread");
    }

    Online::Log::Info("HybridServer started on port " + std::to_string(port));
    return true;
}

void Online::Net::Server::HybridServer::Release()
{
    running.store(false, std::memory_order_release);
    Thread::UnregisterThread(netThreadId);

    // 断开所有连接并释放资源
    {
        std::lock_guard lock(connMutex);
        for (auto& kv : connections)
            ReleaseConnectionResources(kv.second);
        connections.clear();
        peerToConnId.clear();
    }

    if (host) {
        enet_host_destroy(host);
        host = nullptr;
    }
    enet_deinitialize();

    // 清理消息队列
    auto allQueues = messageQueues.ExtractAll();
    for (auto& [key, q] : allQueues) delete q;
    messageQueues.Clear();
}

void Online::Net::Server::HybridServer::Tick()
{
    static const uint32_t HEARTBEAT_TIMEOUT = 30000;  // 30 秒超时
    uint32_t now = Online::Core::CurrentMs();

    std::lock_guard lock(connMutex);
    for (auto it = connections.begin(); it != connections.end(); ) {
        Connection* conn = it->second;
        if (now - conn->lastHeartbeatMs > HEARTBEAT_TIMEOUT) {
            Online::Log::Info("Heartbeat timeout for client " + std::to_string(conn->id));
            ReleaseConnectionResources(conn);
            it = connections.erase(it);
        }
        else {
            ++it;
        }
    }
}

void Online::Net::Server::HybridServer::NetThreadFunc()
{
    ENetEvent event;
    while (running.load(std::memory_order_acquire)) {
        // 阻塞最多 10ms 等待事件
        while (enet_host_service(host, &event, 10) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                HandleConnect(event);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                HandleReceive(event);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                HandleDisconnect(event);
                break;
            default:
                break;
            }
        }
    }
}

void Online::Net::Server::HybridServer::HandleConnect(const ENetEvent& event)
{
    std::lock_guard lock(connMutex);
    AcceptConnection(event.peer);
}

void Online::Net::Server::HybridServer::AcceptConnection(ENetPeer* peer)
{
    int connId = nextConnectionId++;

    Connection* conn = connPool.Get();
    conn->id = connId;
    conn->peer = peer;
    conn->lastHeartbeatMs = Online::Core::CurrentMs();
    conn->handshakeCompleted = false;

    connections[connId] = conn;
    peerToConnId[peer] = connId;

    // 通过可靠有序通道发送 ConnID
    if (!SendClientId(peer, connId)) {
        ReleaseConnectionResources(conn);
        connections.erase(connId);
        peerToConnId.erase(peer);
        return;
    }

    Online::Log::Info("New client connected, assigned ID: " + std::to_string(connId));
}

bool Online::Net::Server::HybridServer::SendClientId(ENetPeer* peer, int connId)
{
    PacketHeader hdr{};
    hdr.type = static_cast<uint16_t>(PacketType::ConnID);
    hdr.length = sizeof(int);

    std::vector<std::byte> packet(sizeof(PacketHeader) + sizeof(int));
    std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
    std::memcpy(packet.data() + sizeof(PacketHeader), &connId, sizeof(int));

    ENetPacket* pkt = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    if (!pkt) return false;
    return enet_peer_send(peer, static_cast<uint8_t>(ChannelType::ReliableOrdered), pkt) == 0;
}

void Online::Net::Server::HybridServer::HandleReceive(const ENetEvent& event)
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

    // 握手包处理
    if (header.type == static_cast<uint16_t>(PacketType::UdpHandshake)) {
        if (header.length != sizeof(int)) {
            enet_packet_destroy(event.packet);
            return;
        }
        int connId = *reinterpret_cast<const int*>(data.data() + sizeof(PacketHeader));

        std::lock_guard lock(connMutex);
        auto it = connections.find(connId);
        if (it != connections.end() && it->second->peer == event.peer && !it->second->handshakeCompleted) {
            it->second->handshakeCompleted = true;
            it->second->lastHeartbeatMs = Online::Core::CurrentMs();
            Online::Log::Info("Handshake completed for client " + std::to_string(connId));
        }
        else {
            // 无效握手，断开连接
            Online::Log::Warning("Invalid handshake attempt, disconnecting peer");
            enet_peer_disconnect(event.peer, 0);
        }
        enet_packet_destroy(event.packet);
        return;
    }

    // 心跳包处理（只更新心跳时间，不产生业务消息）
    if (header.type == static_cast<uint16_t>(PacketType::Heartbeat)) {
        std::lock_guard lock(connMutex);
        auto it = peerToConnId.find(event.peer);
        if (it != peerToConnId.end()) {
            auto connIt = connections.find(it->second);
            if (connIt != connections.end())
                connIt->second->lastHeartbeatMs = Online::Core::CurrentMs();
        }
        enet_packet_destroy(event.packet);
        return;
    }

    // 普通业务消息（必须握手完成）
    {
        std::lock_guard lock(connMutex);
        auto it = peerToConnId.find(event.peer);
        if (it == peerToConnId.end()) {
            enet_packet_destroy(event.packet);
            return;  // 未注册的对端
        }

        int connId = it->second;
        auto connIt = connections.find(connId);
        if (connIt == connections.end() || !connIt->second->handshakeCompleted) {
            enet_packet_destroy(event.packet);
            return;  // 握手未完成
        }

        // 更新心跳时间
        connIt->second->lastHeartbeatMs = Online::Core::CurrentMs();

        NetMessage msg;
        msg.connectionId = connId;
        msg.header = header;
        msg.body.assign(data.begin() + sizeof(PacketHeader), data.end());
        PushMessage(std::move(msg));
    }
    enet_packet_destroy(event.packet);
}

void Online::Net::Server::HybridServer::HandleDisconnect(const ENetEvent& event)
{
    std::lock_guard lock(connMutex);
    auto it = peerToConnId.find(event.peer);
    if (it != peerToConnId.end()) {
        int connId = it->second;
        auto connIt = connections.find(connId);
        if (connIt != connections.end()) {
            ReleaseConnectionResources(connIt->second);
            connections.erase(connIt);
        }
        Online::Log::Info("Client " + std::to_string(connId) + " disconnected");
    }
}

void Online::Net::Server::HybridServer::ReleaseConnectionResources(Connection* conn)
{
    if (!conn) return;
    Online::Log::Info("Releasing client " + std::to_string(conn->id));

    // ENet peer 断开由对方或超时处理，这里只清理本地记录
    if (conn->peer) {
        peerToConnId.erase(conn->peer);
        enet_peer_disconnect(conn->peer, 0);
        conn->peer = nullptr;
    }
    conn->handshakeCompleted = false;
    connPool.Release(conn);
}

void Online::Net::Server::HybridServer::PushMessage(NetMessage&& msg)
{
    GetMessageQueue(static_cast<PacketType>(msg.header.type)).Push(std::move(msg));
}

Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& Online::Net::Server::HybridServer::GetMessageQueue(PacketType type)
{
    uint16_t key = static_cast<uint16_t>(type);
    auto& queuePtr = messageQueues.GetOrCreate(key, []() -> Core::ThreadSafeQueue<NetMessage>*{
        return new Core::ThreadSafeQueue<NetMessage>();
        });
    return *queuePtr;
}

// ---------- 发送接口 ----------

bool Online::Net::Server::HybridServer::SendReliable(int connectionId, std::span<const std::byte> data,
    PacketType type, ChannelType channel)
{
    std::lock_guard lock(connMutex);
    auto it = connections.find(connectionId);
    if (it == connections.end() || !it->second->peer || !it->second->handshakeCompleted)
        return false;

    PacketHeader hdr{};
    hdr.type = static_cast<uint16_t>(type);
    hdr.length = static_cast<uint32_t>(data.size());

    std::vector<std::byte> packet(sizeof(PacketHeader) + data.size());
    std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
    if (!data.empty())
        std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());

    ENetPacket* pkt = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
    if (!pkt) return false;
    return enet_peer_send(it->second->peer, static_cast<uint8_t>(channel), pkt) == 0;
}

bool Online::Net::Server::HybridServer::BroadcastReliable(std::span<const std::byte> data, PacketType type,
    ChannelType channel)
{
    std::lock_guard lock(connMutex);
    bool ok = true;
    for (auto& kv : connections) {
        Connection* conn = kv.second;
        if (!conn->peer || !conn->handshakeCompleted) continue;

        PacketHeader hdr{};
        hdr.type = static_cast<uint16_t>(type);
        hdr.length = static_cast<uint32_t>(data.size());

        std::vector<std::byte> packet(sizeof(PacketHeader) + data.size());
        std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
        if (!data.empty())
            std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());

        ENetPacket* pkt = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_RELIABLE);
        if (!pkt) { ok = false; continue; }
        if (enet_peer_send(conn->peer, static_cast<uint8_t>(channel), pkt) != 0)
            ok = false;
    }
    return ok;
}

bool Online::Net::Server::HybridServer::SendUnreliable(int connectionId, std::span<const std::byte> data,
    PacketType type, ChannelType channel)
{
    std::lock_guard lock(connMutex);
    auto it = connections.find(connectionId);
    if (it == connections.end() || !it->second->peer || !it->second->handshakeCompleted)
        return false;

    PacketHeader hdr{};
    hdr.type = static_cast<uint16_t>(type);
    hdr.length = static_cast<uint32_t>(data.size());

    std::vector<std::byte> packet(sizeof(PacketHeader) + data.size());
    std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
    if (!data.empty())
        std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());

    ENetPacket* pkt = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_UNSEQUENCED);
    if (!pkt) return false;
    return enet_peer_send(it->second->peer, static_cast<uint8_t>(channel), pkt) == 0;
}

bool Online::Net::Server::HybridServer::BroadcastUnreliable(std::span<const std::byte> data,
    PacketType type, ChannelType channel)
{
    std::lock_guard lock(connMutex);
    for (auto& kv : connections) {
        Connection* conn = kv.second;
        if (!conn->peer || !conn->handshakeCompleted) continue;

        PacketHeader hdr{};
        hdr.type = static_cast<uint16_t>(type);
        hdr.length = static_cast<uint32_t>(data.size());

        std::vector<std::byte> packet(sizeof(PacketHeader) + data.size());
        std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
        if (!data.empty())
            std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());

        ENetPacket* pkt = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_UNSEQUENCED);
        if (pkt)
            enet_peer_send(conn->peer, static_cast<uint8_t>(channel), pkt);
    }
    return true;
}

bool Online::Net::Server::HybridServer::BroadcastUnreliableExcept(int excludeConnId,
    std::span<const std::byte> data,
    PacketType type, ChannelType channel)
{
    std::lock_guard lock(connMutex);
    for (auto& kv : connections) {
        if (kv.first == excludeConnId) continue;
        Connection* conn = kv.second;
        if (!conn->peer || !conn->handshakeCompleted) continue;

        PacketHeader hdr{};
        hdr.type = static_cast<uint16_t>(type);
        hdr.length = static_cast<uint32_t>(data.size());

        std::vector<std::byte> packet(sizeof(PacketHeader) + data.size());
        std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
        if (!data.empty())
            std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());

        ENetPacket* pkt = enet_packet_create(packet.data(), packet.size(), ENET_PACKET_FLAG_UNSEQUENCED);
        if (pkt)
            enet_peer_send(conn->peer, static_cast<uint8_t>(channel), pkt);
    }
    return true;
}

void Online::Net::Server::HybridServer::CloseConnection(int connectionId)
{
    std::lock_guard lock(connMutex);
    auto it = connections.find(connectionId);
    if (it != connections.end() && it->second->peer) {
        enet_peer_disconnect(it->second->peer, 0);
        // 清理工作将在 HandleDisconnect 中完成
    }
}

int Online::Net::Server::HybridServer::GetConnectionCount() const
{
    std::lock_guard lock(connMutex);
    return static_cast<int>(connections.size());
}