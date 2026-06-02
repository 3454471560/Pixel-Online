#include<Thread/ThreadTracker.h>

bool Online::Thread::ThreadTracker::Initialize()
{
	registry.Map.emplace(Online::Core::Thread::GetCurrentThreadId(), ThreadInfo("Main"));
	return true;
}
void Online::Thread::ThreadTracker::Release()
{
	auto iterator = registry.Map.begin();
	while (iterator != registry.Map.end())
	{
		if (iterator->second.name == "Main" || iterator->second.name == "Log")
		{
			iterator++;
			continue;
		}

		Online::Log::Error(iterator->second.name + " not actively recycled, " +
			(iterator->second.thread.JoinWaitForMilliseconds(std::chrono::milliseconds(1000)) ? "join success" : "join timeout terminated"));
		iterator = registry.Map.erase(iterator);
	}
}
