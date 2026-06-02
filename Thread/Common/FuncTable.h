#pragma once

#include<Context/Context.h>
#include<Core/Thread/Thread.h>
#include<stdexcept>
#include<string>

namespace Online::Runtime
{
	template<>
	struct FuncTable<Online::Thread::ThreadTracker>
	{
		friend class Online::Runtime::Client;
		friend class Online::Runtime::Server;
	private:
		FuncTable() = default;
		~FuncTable() = default;

	public:
		FuncTable(const FuncTable&) = delete;
		FuncTable& operator=(const FuncTable&) = delete;
		FuncTable(FuncTable&&) = delete;
		FuncTable& operator=(FuncTable&&) = delete;

		inline bool Check() const
		{
			if (!OnRegisterThread) { throw std::runtime_error("FuncTable miss [Thread::RegisterThread] Function!"); }
			if (!OnUnregisterThread) { throw std::runtime_error("FuncTable miss [Thread::UnregisterThread] Function!"); }
			if (!OnGetThreadName) { throw std::runtime_error("FuncTable miss [Thread::GetThreadName] Function!"); }
			if (!OnGetThreadIsRunning) { throw std::runtime_error("FuncTable miss [Thread::GetThreadIsRunning] Function!"); }
			return true;
		}
		inline void UnRegister() noexcept
		{
			OnRegisterThread = nullptr;
			OnUnregisterThread = nullptr;
			OnGetThreadName = nullptr;
			OnGetThreadIsRunning = nullptr;
		}

	public:
		inline Online::Core::Thread::Identifier InvokeOnRegisterThread(std::string_view name, void(*Thread)(void*, void*), void* bootstraper, void* args)
		{
			return OnRegisterThread(name, Thread, bootstraper, args);
		}
		inline bool InvokeOnUnregisterThread(Online::Core::Thread::Identifier id) noexcept
		{
			return OnUnregisterThread(id);
		}
		inline std::string InvokeOnGetThreadName(Online::Core::Thread::Identifier id) noexcept
		{
			return OnGetThreadName(id);
		}
		inline bool InvokeOnGetThreadIsRunning(Online::Core::Thread::Identifier id) noexcept
		{
			return OnGetThreadIsRunning(id);
		}

	private:
		Online::Core::Thread::Identifier(*OnRegisterThread)(std::string_view, void(*)(void*, void*), void*, void*) = nullptr;
		bool (*OnUnregisterThread)(Online::Core::Thread::Identifier) noexcept = nullptr;
		std::string(*OnGetThreadName)(Online::Core::Thread::Identifier) noexcept = nullptr;
		bool (*OnGetThreadIsRunning)(Online::Core::Thread::Identifier) noexcept = nullptr;
	};
}

namespace Online::Thread
{
	inline Online::Core::Thread::Identifier RegisterThread(std::string_view name, void(*Thread)(void*, void*), void* bootstraper, void* args)
	{
		return Online::Runtime::Context::Instance().GetFuncTable<Online::Thread::ThreadTracker>().InvokeOnRegisterThread(name, Thread, bootstraper, args);
	}
	inline bool UnregisterThread(Online::Core::Thread::Identifier id) noexcept
	{
		return Online::Runtime::Context::Instance().GetFuncTable<Online::Thread::ThreadTracker>().InvokeOnUnregisterThread(id);
	}
	inline std::string GetThreadName(Online::Core::Thread::Identifier id) noexcept
	{
		return Online::Runtime::Context::Instance().GetFuncTable<Online::Thread::ThreadTracker>().InvokeOnGetThreadName(id);
	}
	inline bool GetThreadIsRunning(Online::Core::Thread::Identifier id) noexcept
	{
		return Online::Runtime::Context::Instance().GetFuncTable<Online::Thread::ThreadTracker>().InvokeOnGetThreadIsRunning(id);
	}
}