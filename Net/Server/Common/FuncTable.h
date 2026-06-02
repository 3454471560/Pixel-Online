#pragma once
#include<Context/Context.h>
#include<Server/Context/ServerContext.h>
#include<Core/ThreadSafe/ThreadSafeQueue.h>
#include<Net/Common/NetCommon.h>
#include<stdexcept>

namespace Online::Runtime
{
	template<>
	struct FuncTable<Online::Net::Server::NetworkServer>
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
			if (!OnGetMessageQueue) { throw std::runtime_error("Delegates miss [Net::OnGetMessageQueue] Function!"); }
			if (!OnSend) { throw std::runtime_error("Delegates miss [Net::Send] Function!"); }
			if (!OnBroadcast) { throw std::runtime_error("Delegates miss [Net::Broadcast] Function!"); }
			return true;
		}
		inline void UnRegister() noexcept
		{
			OnGetMessageQueue = nullptr;
			OnSend = nullptr;
			OnBroadcast = nullptr;
		}

	public:
		inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& InvokeOnGetMessageQueue() noexcept
		{
			return OnGetMessageQueue();
		}

		inline bool InvokeOnSend(int connectionId, std::span<const std::byte> data) noexcept
		{
			return OnSend(connectionId, data);
		}

		inline bool InvokeOnBroadcast(std::span<const std::byte> data) noexcept
		{
			return OnBroadcast(data);
		}

	private:
		Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& (*OnGetMessageQueue)() noexcept = nullptr;
		bool (*OnSend)(int, std::span<const std::byte>) noexcept = nullptr;
		bool (*OnBroadcast)(std::span<const std::byte>) noexcept = nullptr;
	};
}

namespace Online::Net::Server
{
	inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& GetMessageQueue() noexcept
	{
		return Online::Runtime::ServerContext::Instance().GetServerFuncTable<Online::Net::Server::NetworkServer>().InvokeOnGetMessageQueue();
	}

	inline bool Send(int connectionId, std::span<const std::byte> data) noexcept
	{
		return Online::Runtime::ServerContext::Instance().GetServerFuncTable<Online::Net::Server::NetworkServer>().InvokeOnSend(connectionId, data);
	}

	inline bool Send(std::span<const std::byte> data) noexcept
	{
		return Online::Runtime::ServerContext::Instance().GetServerFuncTable<Online::Net::Server::NetworkServer>().InvokeOnBroadcast(data);
	}
}
