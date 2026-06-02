#pragma once

#include<Core/Allocate/Allocate.h>
#include<Core/Thread/Thread.h>
#include<Log/Common/FuncTable.h>

#include<string>
#include<unordered_map>
#include<optional>
#include<thread>
#include<sstream>

namespace Online::Runtime { class Runtime; }

namespace Online::Thread
{
	class ThreadTracker
	{
	public:
		struct Factory
		{
			friend class Online::Runtime::Module<ThreadTracker>;
		private:
			static ThreadTracker* Create()
			{
				return ONLINE_NEW(ThreadTracker);
			}
			static void Destroy(ThreadTracker* threadTracker)
			{
				ONLINE_DELETE(threadTracker);
			}
		};
		struct Lifecycle
		{
			friend class Online::Runtime::Module<ThreadTracker>;
		private:
			static bool Initialize(ThreadTracker* threadTracker)
			{
				return threadTracker->Initialize();
			}
			static void Release(ThreadTracker* threadTracker)
			{
				threadTracker->Release();
			}
		};

	private:
		struct ThreadInfo
		{
			template<class Func, class... Args>
			ThreadInfo(std::string_view name, Func&& func, Args&&... args)
				:name(name), thread(std::forward<Func>(func), std::forward<Args>(args)...) {
			}
			ThreadInfo(std::string_view name)
				:name(name) {
			}
			~ThreadInfo() = default;
			ThreadInfo(const ThreadInfo&) = delete;
			ThreadInfo& operator=(const ThreadInfo&) = delete;
			ThreadInfo(ThreadInfo&&) noexcept = default;
			ThreadInfo& operator=(ThreadInfo&&) noexcept = default;

			std::string name;
			Online::Core::Thread thread;
		};
		struct ThreadRegistry
		{
			std::unordered_map<Online::Core::Thread::Identifier,
				ThreadInfo> Map;
		};

	private:
		ThreadTracker() = default;
		~ThreadTracker() = default;

	public:
		ThreadTracker(const ThreadTracker&) = delete;
		ThreadTracker& operator=(const ThreadTracker&) = delete;
		ThreadTracker(ThreadTracker&&) = delete;
		ThreadTracker& operator=(ThreadTracker&&) = delete;

	private:
		bool Initialize();
		void Release();

	public:
		template<class Func, class... Args>
		inline Online::Core::Thread::Identifier RegisterThread(std::string_view name, Func&& func, Args&&... args)
		{
			ThreadInfo threadInfo = ThreadInfo(name, std::forward<Func>(func), std::forward<Args>(args)...);
			Online::Core::Thread::Identifier id = threadInfo.thread.GetThreadId();
			registry.Map.emplace(id, std::move(threadInfo));
			Online::Log::Info(std::string(name) + " thread created and registered");
			return id;
		}
		inline bool UnregisterThread(Online::Core::Thread::Identifier id)
		{
			auto iterator = registry.Map.find(id);
			if (iterator == registry.Map.end())
			{
				Online::Log::Error("unregistered uhread id:" + std::to_string(id));
				return false;
			}

			if (iterator->second.thread.Join())
			{
				Online::Log::Info(GetThreadName(id) + " thread join success");
				registry.Map.erase(iterator);
				return true;
			}

			Online::Log::Error(GetThreadName(id) + " thread join fail");
			registry.Map.erase(iterator);
			return false;
		}
		inline bool UnregisterThread(Online::Core::Thread::Identifier id, const std::chrono::milliseconds& timeout)
		{
			auto iterator = registry.Map.find(id);
			if (iterator == registry.Map.end())
			{
				Online::Log::Error("unregistered thread id:" + std::to_string(id));
				return false;
			}

			if (iterator->second.thread.JoinWaitForMilliseconds(timeout))
			{
				Online::Log::Info(GetThreadName(id) + " thread join success");
				registry.Map.erase(iterator);
				return true;
			}

			Online::Log::Error(GetThreadName(id) + " thread join timeout and terminated");
			registry.Map.erase(iterator);
			return false;
		}
		inline std::string GetThreadName(Online::Core::Thread::Identifier id)
		{
			auto iterator = registry.Map.find(id);
			return iterator == registry.Map.end() ?
				"try find unregistered thread id:" + std::to_string(id) :
				iterator->second.name;
		}
		inline bool GetThreadIsRunning(Online::Core::Thread::Identifier id)
		{
			auto iterator = registry.Map.find(id);
			if (iterator == registry.Map.end())
			{
				Online::Log::Error("try find unregistered thread id:" + std::to_string(id));
				return false;
			}

			return iterator->second.thread.IsRunning();
		}

	private:
		ThreadRegistry registry;
	};
}
