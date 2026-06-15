#include<Task/TaskScheduler.h>
#include<Thread/Common/FuncTable.h>

#include<algorithm>

bool Online::Task::TaskScheduler::Initialize()
{
	uint32_t cpuCoreCount = static_cast<uint32_t>(std::thread::hardware_concurrency());
	cpuCoreCount = cpuCoreCount == 0 ? 4 : cpuCoreCount;

	uint32_t jobExecuterCount = static_cast<uint32_t>(cpuCoreCount * 0.6f) - 5;
	jobExecuterCount = std::clamp(jobExecuterCount, static_cast<uint32_t>(2), static_cast<uint32_t>(cpuCoreCount / 2));

	jobThreads.resize(jobExecuterCount);
	isRunning.store(true, std::memory_order_release);
	for (size_t i = 0; i < jobExecuterCount; i++)
	{
		jobThreads[i] = Online::Thread::RegisterThread
		("JobThread_" + std::to_string(i + 1), &Online::Task::TaskScheduler::BootstrapJobThread, this, nullptr);
	}
	return true;
}
void Online::Task::TaskScheduler::Release()
{
	isRunning.store(false, std::memory_order_release);
	cond.notify_all();
	for (auto token : jobThreads)
	{
		Online::Thread::UnregisterThread(token);
	}
	jobThreads.clear();
}
