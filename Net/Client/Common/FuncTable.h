#pragma once
#include<Context/Context.h>
#include<Client/Context/ClientContext.h>
#include<Core/ThreadSafe/ThreadSafeQueue.h>
#include<Net/Common/NetCommon.h>
#include<stdexcept>

namespace Online::Runtime
{
	template<>
	struct FuncTable<Online::Net::Client::NetworkClient>
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
			if (!OnConnect) { throw std::runtime_error("Delegates miss [Net::OnConnect] Function!"); }
			if (!OnDisconnect) { throw std::runtime_error("Delegates miss [Net::OnDisonnect] Function!"); }
			if (!OnGetMessageQueue) { throw std::runtime_error("Delegates miss [Net::OnGetMessageQueue] Function!"); }
			if (!OnSend) { throw std::runtime_error("Delegates miss [Net::Send] Function!"); }
			if (!OnIsConnected) { throw std::runtime_error("Delegates miss [Net::OnIsConnected] Function!"); }
			return true;
		}
		inline void UnRegister() noexcept
		{
			OnConnect = nullptr;
			OnDisconnect = nullptr;
			OnGetMessageQueue = nullptr;
			OnSend = nullptr;
			OnIsConnected = nullptr;
		}

	public:
		inline bool InvokeOnConnect(const std::string& host, uint16_t port) noexcept
		{
			return OnConnect(host, port);
		}
		inline void InvokeOnDisconnect() noexcept
		{
			OnDisconnect();
		}
		inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& InvokeOnGetMessageQueue() noexcept
		{
			return OnGetMessageQueue();
		}

		inline bool InvokeOnSend(std::span<const std::byte> data) noexcept
		{
			return OnSend(data);
		}

		inline bool InvokeOnIsConnected() noexcept
		{
			return OnIsConnected();
		}

	private:
		bool (*OnConnect)(const std::string&, uint16_t) noexcept = nullptr;
		void (*OnDisconnect)() noexcept = nullptr;
		Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& (*OnGetMessageQueue)() noexcept = nullptr;
		bool (*OnSend)(std::span<const std::byte>) noexcept = nullptr;
		bool (*OnIsConnected)() noexcept = nullptr;
	};
}

namespace Online::Net::Server
{
	inline bool Connect(const std::string& ip, uint16_t port) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Net::Client::NetworkClient>().InvokeOnConnect(ip, port);
	}
	inline void DisConnect() noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Net::Client::NetworkClient>().InvokeOnDisconnect();
	}
	inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& GetMessageQueue() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Net::Client::NetworkClient>().InvokeOnGetMessageQueue();
	}

	inline bool Send(int connectionId, std::span<const std::byte> data) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Net::Client::NetworkClient>().InvokeOnSend(data);
	}

	inline bool IsConnected() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Net::Client::NetworkClient>().InvokeOnIsConnected();
	}
}
