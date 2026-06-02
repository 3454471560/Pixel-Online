#include <Core/Time/time.h>
#include <Log/Common/FuncTable.h>
#include <Net/Client/NetworkClient.h>

#include <cstring>
#include <stdexcept>

bool Online::Net::Client::NetworkClient::Initialize()
{
    if (SDLNet_Init() == -1) { throw std::runtime_error("SDLNet_Init failed: " + std::string(SDLNet_GetError())); }

    readSet = SDLNet_AllocSocketSet(16);
    writeSet = SDLNet_AllocSocketSet(16);

    if (!readSet || !writeSet) 
    {
        if (readSet) SDLNet_FreeSocketSet(readSet);
        if (writeSet) SDLNet_FreeSocketSet(writeSet);
        SDLNet_Quit();
        throw std::runtime_error("SDLNet_AllocSocketSet failed: " + std::string(SDLNet_GetError()));
    }

    isRunning.store(true, std::memory_order_release);
    netThread = Online::Thread::RegisterThread("NetClientThread", BootstrapNetThread, this, nullptr);
    if (netThread == Core::Thread::Identifier::Invalid) { throw std::runtime_error("Failed to register network client thread"); }

    return true;
}

void Online::Net::Client::NetworkClient::Release()
{
    Disconnect();

    isRunning.store(false, std::memory_order_release);
    Thread::UnregisterThread(netThread);

    {
        std::lock_guard lock(connMutex);
        if (connected) 
        {
            ReleaseConnectionResources();
            connected = false;
        }
    }

    if (readSet) {
        SDLNet_FreeSocketSet(readSet);
        readSet = nullptr;
    }
    if (writeSet) {
        SDLNet_FreeSocketSet(writeSet);
        writeSet = nullptr;
    }

    SDLNet_Quit();
}

bool Online::Net::Client::NetworkClient::Connect(const std::string& host, uint16_t port)
{
    std::lock_guard lock(connMutex);

    if (connected.load(std::memory_order_acquire))
    {
        ReleaseConnectionResources();
        connected = false;
    }

    IPaddress serverAddr;
    if (SDLNet_ResolveHost(&serverAddr, host.c_str(), port) == -1) 
    {
        Online::Log::Error("ResolveHost failed: " + std::string(SDLNet_GetError()));
        return false;
    }

    TCPsocket newSock = SDLNet_TCP_Open(&serverAddr);
    if (!newSock) 
    {
        Online::Log::Error("TCP_Open failed: " + std::string(SDLNet_GetError()));
        return false;
    }

    PacketHeader idHeader;
    if (!RecvAll(newSock, &idHeader, sizeof(PacketHeader))) 
    {
        Online::Log::Error("Failed to receive ID header");
        SDLNet_TCP_Close(newSock);
        return false;
    }

    if (idHeader.length != sizeof(int)) 
    {
        Online::Log::Error("Invalid ID packet length");
        SDLNet_TCP_Close(newSock);
        return false;
    }

    int assignedId = -1;
    if (!RecvAll(newSock, &assignedId, sizeof(int))) 
    {
        Online::Log::Error("Failed to receive connection ID");
        SDLNet_TCP_Close(newSock);
        return false;
    }

    connection.Reset();
    connection.id = assignedId;
    connection.socket = newSock;
    connection.lastHeartbeatMs = Online::Core::CurrentMs();
    connection.inWriteSet = false;

    if (SDLNet_TCP_AddSocket(readSet, newSock) == -1) 
    {
        Online::Log::Error("AddSocket to readSet failed");
        SDLNet_TCP_Close(newSock);
        return false;
    }

    connected.store(true, std::memory_order_release);
    Online::Log::Info("Connected to server " + host + ":" + std::to_string(port) + " with ID: " + std::to_string(assignedId));
    return true;
}

void Online::Net::Client::NetworkClient::Disconnect()
{
    std::lock_guard lock(connMutex);
    if (connected) 
    {
        ReleaseConnectionResources();
        connected = false;
        Online::Log::Info("Disconnected from server");
    }
}

void Online::Net::Client::NetworkClient::ReleaseConnectionResources()
{
    if (!connection.socket) return;

    Online::Log::Info("Releasing connection resources");

    SDLNet_TCP_DelSocket(readSet, connection.socket);
    if (connection.inWriteSet) {
        SDLNet_TCP_DelSocket(writeSet, connection.socket);
    }

    SDLNet_TCP_Close(connection.socket);

    while (!connection.sendQueue.empty()) connection.sendQueue.pop();
    connection.Reset();
}

void Online::Net::Client::NetworkClient::CloseConnection()
{
    std::lock_guard lock(connMutex);
    if (connected) {
        ReleaseConnectionResources();
        connected = false;
    }
}

bool Online::Net::Client::NetworkClient::Send(std::span<const std::byte> data)
{
    std::lock_guard lock(connMutex);
    return SendLocked(data);
}

bool Online::Net::Client::NetworkClient::IsConnected() const
{
    return connected;
}

bool Online::Net::Client::NetworkClient::SendLocked(std::span<const std::byte> data)
{
    if (!connected) return false;

    PacketHeader hdr{};
    hdr.length = static_cast<uint32_t>(data.size());

    std::vector<std::byte> packet;
    packet.resize(sizeof(PacketHeader) + data.size());
    std::memcpy(packet.data(), &hdr, sizeof(PacketHeader));
    if (!data.empty()) 
    {
        std::memcpy(packet.data() + sizeof(PacketHeader), data.data(), data.size());
    }

    if (connection.sendQueue.size() >= 64) 
    {
        Online::Log::Warning("Send queue overflow, dropping packet");
        return false;
    }

    connection.sendQueue.push(std::move(packet));

    if (!connection.inWriteSet) 
    {
        SDLNet_TCP_AddSocket(writeSet, connection.socket);
        connection.inWriteSet = true;
    }

    return true;
}

bool Online::Net::Client::NetworkClient::RecvAll(TCPsocket sock, void* buffer, int len)
{
    auto ptr = reinterpret_cast<uint8_t*>(buffer);
    int remaining = len;
    while (remaining > 0)
    {
        int got = SDLNet_TCP_Recv(sock, ptr, remaining);
        if (got <= 0) return false;
        ptr += got;
        remaining -= got;
    }
    return true;
}

bool Online::Net::Client::NetworkClient::TryReceiveNonBlocking()
{
    constexpr size_t CHUNK_SIZE = 4096;
    size_t prevSize = connection.recvBuffer.size();
    connection.recvBuffer.resize(prevSize + CHUNK_SIZE);

    int got = SDLNet_TCP_Recv(connection.socket,
        connection.recvBuffer.data() + prevSize,
        static_cast<int>(CHUNK_SIZE));
    if (got > 0) 
    {
        connection.recvBuffer.resize(prevSize + got);
        connection.lastHeartbeatMs = Online::Core::CurrentMs();
        return true;
    }
    else if (got == 0) 
    {
        connection.recvBuffer.resize(prevSize);
        return false;
    }
    else 
    {
        connection.recvBuffer.resize(prevSize);
        return true;
    }
}

bool Online::Net::Client::NetworkClient::ParseMessages()
{
    while (connection.recvBuffer.size() >= sizeof(PacketHeader))
    {
        PacketHeader header;
        std::memcpy(&header, connection.recvBuffer.data(), sizeof(PacketHeader));

        if (header.length > 1024 * 1024) 
        {
            Online::Log::Warning("Received oversized packet: " + std::to_string(header.length));
            return false;
        }

        size_t totalSize = sizeof(PacketHeader) + header.length;
        if (connection.recvBuffer.size() < totalSize)
            break;

        NetMessage msg;
        msg.connectionId = 0;
        msg.header = header;
        msg.body.assign(connection.recvBuffer.begin() + sizeof(PacketHeader),
            connection.recvBuffer.begin() + totalSize);
        messages.Push(std::move(msg));

        connection.recvBuffer.erase(connection.recvBuffer.begin(),
            connection.recvBuffer.begin() + totalSize);
    }
    return true;
}

void Online::Net::Client::NetworkClient::Tick()
{
    static const uint32_t HEARTBEAT_TIMEOUT = 30000;
    uint32_t now = Online::Core::CurrentMs();

    std::lock_guard lock(connMutex);
    if (connected && (now - connection.lastHeartbeatMs > HEARTBEAT_TIMEOUT))
    {
        Online::Log::Info("Heartbeat timeout, disconnecting");
        ReleaseConnectionResources();
        connected = false;
    }
}

void Online::Net::Client::NetworkClient::NetThread()
{
    while (isRunning.load(std::memory_order_acquire))
    {
        int readReady = SDLNet_CheckSockets(readSet, 10);
        if (readReady < 0) continue;

        int writeReady = SDLNet_CheckSockets(writeSet, 0);
        if (writeReady < 0) writeReady = 0;

        std::lock_guard lock(connMutex);

        if (!connected) continue;

        if (readReady > 0 && SDLNet_SocketReady(connection.socket))
        {
            if (!TryReceiveNonBlocking())
            {
                Online::Log::Info("Server closed the connection");
                ReleaseConnectionResources();
                connected = false;
                continue;
            }
            else if (!ParseMessages())
            {
                Online::Log::Warning("Packet parse error, disconnecting");
                ReleaseConnectionResources();
                connected = false;
                continue;
            }
        }

        if (connected && writeReady > 0 && connection.inWriteSet && SDLNet_SocketReady(connection.socket))
        {
            const auto& packet = connection.sendQueue.front();
            const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(packet.data()) + connection.sendOffset;
            int remaining = static_cast<int>(packet.size() - connection.sendOffset);

            int sent = SDLNet_TCP_Send(connection.socket, dataPtr, remaining);

            if (sent > 0)
            {
                connection.sendOffset += sent;
                if (connection.sendOffset == packet.size())
                {
                    connection.sendQueue.pop();
                    connection.sendOffset = 0;
                }
                connection.lastHeartbeatMs = Online::Core::CurrentMs();

                if (connection.sendQueue.empty())
                {
                    SDLNet_TCP_DelSocket(writeSet, connection.socket);
                    connection.inWriteSet = false;
                }
            }
            else if (sent < 0 || sent == 0)
            {
                Online::Log::Error("Send failed, disconnecting");
                ReleaseConnectionResources();
                connected = false;
            }
        }
    }
}
