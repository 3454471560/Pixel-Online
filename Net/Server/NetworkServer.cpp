#include <Core/Time/time.h>
#include <Log/Common/FuncTable.h>
#include <Net/Server/NetworkServer.h>

#include <cstring>
#include <stdexcept>

bool Online::Net::Server::NetworkServer::Initialize()
{
    if (SDLNet_Init() == -1) { throw std::runtime_error("SDLNet_Init failed: " + std::string(SDLNet_GetError())); }

    readSet = SDLNet_AllocSocketSet(1024);
    writeSet = SDLNet_AllocSocketSet(1024);

    if (!readSet || !writeSet) 
    {
        if (readSet) SDLNet_FreeSocketSet(readSet);
        if (writeSet) SDLNet_FreeSocketSet(writeSet);
        SDLNet_Quit();
        throw std::runtime_error("SDLNet_AllocSocketSet failed: " + std::string(SDLNet_GetError()));
    }

    IPaddress serverAddr;
    if (SDLNet_ResolveHost(&serverAddr, nullptr, port) == -1)
    {
        SDLNet_FreeSocketSet(readSet);
        SDLNet_FreeSocketSet(writeSet);
        SDLNet_Quit();
        throw std::runtime_error("SDLNet_ResolveHost failed: " + std::string(SDLNet_GetError()));
    }

    serverSocket = SDLNet_TCP_Open(&serverAddr);
    if (!serverSocket) 
    {
        SDLNet_FreeSocketSet(readSet);
        SDLNet_FreeSocketSet(writeSet);
        SDLNet_Quit();
        throw std::runtime_error("SDLNet_TCP_Open failed on port " + std::to_string(port) + ": " + SDLNet_GetError());
    }

    if (SDLNet_TCP_AddSocket(readSet, serverSocket) == -1) {
        SDLNet_TCP_Close(serverSocket);
        SDLNet_FreeSocketSet(readSet);
        SDLNet_FreeSocketSet(writeSet);
        SDLNet_Quit();
        throw std::runtime_error("SDLNet_TCP_AddSocket failed for server: " + std::string(SDLNet_GetError()));
    }
    return true;
}

void Online::Net::Server::NetworkServer::Release()
{
    {
        std::lock_guard lock(connMutex);
        for (auto& kv : connections)
        {
            ReleaseConnectionResources(kv.second);
        }
        connections.clear();
    }

    if (serverSocket)
    {
        SDLNet_TCP_DelSocket(readSet, serverSocket);
        SDLNet_TCP_Close(serverSocket);
        serverSocket = nullptr;
    }
    if (readSet)
    {
        SDLNet_FreeSocketSet(readSet);
        readSet = nullptr;
    }
    if (writeSet)
    {
        SDLNet_FreeSocketSet(writeSet);
        writeSet = nullptr;
    }

    SDLNet_Quit();
}

void Online::Net::Server::NetworkServer::ReleaseConnectionResources(Online::Net::Connection* conn)
{
    if (!conn || !conn->socket) return;

    Online::Log::Info("Client Disconnect ID: " + std::to_string(conn->id));

    SDLNet_TCP_DelSocket(readSet, conn->socket);
    if (conn->inWriteSet) 
    {
        SDLNet_TCP_DelSocket(writeSet, conn->socket);
    }

    SDLNet_TCP_Close(conn->socket);

    while (!conn->sendQueue.empty()) conn->sendQueue.pop();
    conn->sendOffset = 0;
    conn->recvBuffer.clear();
    conn->inWriteSet = false;

    connPool.Release(conn);
}

void Online::Net::Server::NetworkServer::CloseConnection(int connectionId)
{
    std::lock_guard lock(connMutex);
    auto it = connections.find(connectionId);
    if (it != connections.end())
    {
        ReleaseConnectionResources(it->second);
        connections.erase(it);
    }
}

void Online::Net::Server::NetworkServer::AcceptNewConnection()
{
    TCPsocket clientSock = SDLNet_TCP_Accept(serverSocket);
    if (!clientSock) return;

    int connId = nextConnectionId++;
    Connection* conn = connPool.Get();

    conn->id = connId;
    conn->socket = clientSock;
    conn->lastHeartbeatMs = Online::Core::CurrentMs();
    conn->inWriteSet = false;

    PacketHeader idHdr{};
    idHdr.length = sizeof(int);
    std::vector<std::byte> idPacket(sizeof(PacketHeader) + sizeof(int));
    std::memcpy(idPacket.data(), &idHdr, sizeof(PacketHeader));
    std::memcpy(idPacket.data() + sizeof(PacketHeader), &connId, sizeof(int));

    int sent = SDLNet_TCP_Send(clientSock, idPacket.data(), static_cast<int>(idPacket.size()));
    if (sent != static_cast<int>(idPacket.size())) 
    {
        Online::Log::Error("Failed to send ID to client " + std::to_string(connId));
        SDLNet_TCP_Close(clientSock);
        connPool.Release(conn);
        return;
    }

    if (SDLNet_TCP_AddSocket(readSet, clientSock) == -1) 
    {
        Online::Log::Error("Failed to add client socket to read set");
        SDLNet_TCP_Close(clientSock);
        connPool.Release(conn);
        return;
    }

    connections[connId] = conn;
    Online::Log::Info("New Client Connected ID: " + std::to_string(connId));
}

bool Online::Net::Server::NetworkServer::TryReceiveNonBlocking(Connection* conn)
{
    constexpr size_t CHUNK_SIZE = 4096;

    size_t prevSize = conn->recvBuffer.size();
    conn->recvBuffer.resize(prevSize + CHUNK_SIZE);

    int got = SDLNet_TCP_Recv(conn->socket,
        conn->recvBuffer.data() + prevSize,
        static_cast<int>(CHUNK_SIZE));
    if (got > 0) 
    {
        conn->recvBuffer.resize(prevSize + got);
        conn->lastHeartbeatMs = Online::Core::CurrentMs();
        return true;
    }
    else 
    {
        conn->recvBuffer.resize(prevSize);
        return false;
    }
}

bool Online::Net::Server::NetworkServer::ParseMessages(Connection* conn)
{
    while (conn->recvBuffer.size() >= sizeof(PacketHeader))
    {
        PacketHeader header;
        std::memcpy(&header, conn->recvBuffer.data(), sizeof(PacketHeader));

        if (header.length > 1024 * 1024) 
        {
            Online::Log::Warning("Client " + std::to_string(conn->id) + " sent oversized packet: " + std::to_string(header.length));
            return false;
        }

        size_t totalSize = sizeof(PacketHeader) + header.length;
        if (conn->recvBuffer.size() < totalSize)
            break;

        NetMessage msg;
        msg.connectionId = conn->id;
        msg.header = header;
        msg.body.assign(conn->recvBuffer.begin() + sizeof(PacketHeader),
            conn->recvBuffer.begin() + totalSize);
        messages.Push(std::move(msg));

        conn->recvBuffer.erase(conn->recvBuffer.begin(),
            conn->recvBuffer.begin() + totalSize);
    }
    return true;
}

bool Online::Net::Server::NetworkServer::SendLocked(int connectionId, std::span<const std::byte> data)
{
    auto it = connections.find(connectionId);
    if (it == connections.end()) return false;

    Connection* conn = it->second;

    PacketHeader hdr{};
    hdr.length = static_cast<uint32_t>(data.size());

    std::vector<std::byte> packet;
    packet.resize(sizeof(PacketHeader) + data.size());
    std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
    if (!data.empty()) 
    {
        std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());
    }

    if (conn->sendQueue.size() >= 64) 
    {
        Online::Log::Warning("Client " + std::to_string(conn->id) + " send queue overflow, dropping packet");
        return false;
    }

    conn->sendQueue.push(std::move(packet));

    if (!conn->inWriteSet) 
    {
        SDLNet_TCP_AddSocket(writeSet, conn->socket);
        conn->inWriteSet = true;
    }

    return true;
}

bool Online::Net::Server::NetworkServer::Send(int connectionId, std::span<const std::byte> data)
{
    std::lock_guard lock(connMutex);
    return SendLocked(connectionId, data);
}

bool Online::Net::Server::NetworkServer::Broadcast(std::span<const std::byte> data)
{
    std::lock_guard lock(connMutex);
    bool ok = true;
    for (auto& kv : connections)
    {
        if (!SendLocked(kv.first, data))
            ok = false;
    }
    return ok;
}

void Online::Net::Server::NetworkServer::Tick()
{
    static const uint32_t HEARTBEAT_TIMEOUT = 30000;
    uint32_t now = Online::Core::CurrentMs();

    std::lock_guard lock(connMutex);
    for (auto it = connections.begin(); it != connections.end(); )
    {
        Connection* conn = it->second;
        if (now - conn->lastHeartbeatMs > HEARTBEAT_TIMEOUT)
        {
            Online::Log::Info("Heartbeat timeout for client " + std::to_string(conn->id));
            ReleaseConnectionResources(conn);
            it = connections.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Online::Net::Server::NetworkServer::NetThread()
{
    int readReady = SDLNet_CheckSockets(readSet, 10);
    if (readReady < 0) return;

    int writeReady = SDLNet_CheckSockets(writeSet, 0);
    if (writeReady < 0) writeReady = 0;

    std::lock_guard lock(connMutex);

    if (SDLNet_SocketReady(serverSocket))
    {
        AcceptNewConnection();
    }

    if (readReady > 0)
    {
        for (auto it = connections.begin(); it != connections.end(); )
        {
            Connection* conn = it->second;
            bool erase = false;

            if (SDLNet_SocketReady(conn->socket))
            {
                if (!TryReceiveNonBlocking(conn))
                {
                    ReleaseConnectionResources(conn);
                    erase = true;
                }
                else if (!ParseMessages(conn))
                {
                    ReleaseConnectionResources(conn);
                    erase = true;
                }
            }

            if (erase)
                it = connections.erase(it);
            else
                ++it;
        }
    }

    if (writeReady > 0)
    {
        for (auto it = connections.begin(); it != connections.end(); )
        {
            Connection* conn = it->second;
            bool erase = false;

            if (conn->inWriteSet && SDLNet_SocketReady(conn->socket))
            {
                const auto& packet = conn->sendQueue.front();
                const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(packet.data()) + conn->sendOffset;
                int remaining = static_cast<int>(packet.size() - conn->sendOffset);

                int sent = SDLNet_TCP_Send(conn->socket, dataPtr, remaining);

                if (sent > 0)
                {
                    conn->sendOffset += sent;
                    if (conn->sendOffset == packet.size())
                    {
                        conn->sendQueue.pop();
                        conn->sendOffset = 0;
                    }
                    conn->lastHeartbeatMs = Online::Core::CurrentMs();

                    if (conn->sendQueue.empty())
                    {
                        SDLNet_TCP_DelSocket(writeSet, conn->socket);
                        conn->inWriteSet = false;
                    }
                }
                else if (sent < 0 || sent == 0)
                {
                    Online::Log::Error("Send failed/closed for client " + std::to_string(conn->id));
                    ReleaseConnectionResources(conn);
                    erase = true;
                }
            }

            if (erase)
                it = connections.erase(it);
            else
                ++it;
        }
    }
}