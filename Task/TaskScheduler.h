#pragma once

#include<Core/Thread/Thread.h>
#include<Core/Allocate/Allocate.h>
#include<Context/Common/Module.h>
#include<Log/Common/FuncTable.h>

#include<queue>
#include<mutex>
#include<functional>

namespace Online::Runtime { class Runtime; }

namespace Online::Task
{
	class TaskScheduler
	{
	public:
		struct Factory
		{
			friend class Online::Runtime::Module<TaskScheduler>;
		private:
			static TaskScheduler* Create()
			{
				return ONLINE_NEW(TaskScheduler);
			}
			static void Destroy(TaskScheduler* taskScheduler)
			{
				ONLINE_DELETE(taskScheduler);
			}
		};
		struct Lifecycle
		{
			friend class Online::Runtime::Module<TaskScheduler>;
		private:
			static bool Initialize(TaskScheduler* taskScheduler)
			{
				return taskScheduler->Initialize();
			}
			static void Release(TaskScheduler* taskScheduler)
			{
				taskScheduler->Release();
			}
		};

	private:
		struct Job
		{
			explicit Job(std::function<void()> func, std::string_view name = "Unknown")
				:func(std::move(func)), name(name) {
			}
			Job() = default;
			Job(const Job&) = delete;
			Job& operator=(const Job&) = delete;
			Job(Job&&) = default;
			Job& operator=(Job&&) = default;
			void operator()()
			{
				if (!func)
				{
					Online::Log::Error(name + " Job Is Empty!");
					return;
				}

				Online::Log::Info(name + " Job Star!");
				func();
				Online::Log::Info(name + " Job Over!");
			}

		private:
			std::function<void()> func;
			std::string name = "Unknown";
		};

	private:
		TaskScheduler() = default;
		~TaskScheduler() = default;

	public:
		TaskScheduler(const TaskScheduler&) = delete;
		TaskScheduler& operator=(const TaskScheduler&) = delete;
		TaskScheduler(TaskScheduler&&) = delete;
		TaskScheduler& operator=(TaskScheduler&&) = delete;

	private:
		bool Initialize();
		void Release();

	public:
		inline void PostJob(std::function<void()> func, std::string_view name = "Unknown")
		{
			Online::Log::Info(std::string(name) + " Job is Posted!");
			{
				std::lock_guard<std::mutex> lock(mutex);
				jobs.emplace(std::move(func), name);
			}
			cond.notify_one();
		}

	private:
		inline static void BootstrapJobThread(void* task, void*)
		{
			static_cast<Online::Task::TaskScheduler*>(task)->JobThread();
		}
		inline void JobThread()
		{
			while (true)
			{
				Job job;
				{
					std::unique_lock<std::mutex> lock(mutex);
					cond.wait(lock, [this] {return !jobs.empty() || !isRunning.load(std::memory_order_acquire); });

					if (!isRunning.load(std::memory_order_acquire) && jobs.empty()) { break; }

					job = std::move(jobs.front());
					jobs.pop();
				}
				job();
			}
		}

	private:
		std::queue<Job> jobs;
		mutable std::mutex mutex;
		std::condition_variable cond;
		std::vector<Online::Core::Thread::Identifier> jobThreads;
		std::atomic<bool> isRunning = false;
	};
}
