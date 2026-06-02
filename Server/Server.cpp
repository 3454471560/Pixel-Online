#include <Server/Boostrap.h>
#include <Server/Server.h>
#include <Time/Chronometer.h>
#include <Log/Logger.h>
#include <Thread/ThreadTracker.h>
#include <Event/EventDispatcher.h>
#include <Phys/Frontend/PhysicsSimulator.h>
#include <Task/TaskScheduler.h>
#include <Net/Server/NetworkServer.h>

#include <stdexcept>
#include <string_view>

namespace
{
#pragma region Time
    inline float Ondelta() noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Time::Chronometer>()->Getdelta();
    }
    inline float OnFixdelta() noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Time::Chronometer>()->GetFixdelta();
    }
    inline float OnUnscaledDelta() noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Time::Chronometer>()->GetUnscaledDelta();
    }
    inline float OnTimeScale() noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Time::Chronometer>()->GetTimeScale();
    }
    inline float OnFramerate() noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Time::Chronometer>()->GetFramerate();
    }
    inline double OnSeconds() noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Time::Chronometer>()->Seconds();
    }
    inline int64_t OnMilliseconds() noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Time::Chronometer>()->Milliseconds();
    }
#pragma endregion

#pragma region Log
    inline void OnLog(Online::Log::LogLevel level, std::string_view info) noexcept 
    {
        Online::Runtime::Context::Instance().GetModule<Online::Log::Logger>()->Write(level, info);
    }
#pragma endregion

#pragma region Thread
    inline bool OnUnregisterThread(Online::Core::Thread::Identifier id) noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Thread::ThreadTracker>()->UnregisterThread(id);
    }
    inline std::string OnGetThreadName(Online::Core::Thread::Identifier id) noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Thread::ThreadTracker>()->GetThreadName(id);
    }
    inline bool OnGetThreadIsRunning(Online::Core::Thread::Identifier id) noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Thread::ThreadTracker>()->GetThreadIsRunning(id);
    }
    inline Online::Core::Thread::Identifier OnRegisterThread(std::string_view name, void(*Thread)(void*, void*), void* bootstraper, void* args)
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Thread::ThreadTracker>()->RegisterThread(name, Thread, bootstraper, args);
    }
#pragma endregion

#pragma region Event
    inline Online::Event::EventToken OnSubscribe(Online::Event::EventType type,void(*OnCallBack)(void*, const ::Online::Event::Event&),void* listener) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Event::EventDispatcher>()->Subscribe(type, OnCallBack, listener);
    }
    inline bool OnUnSubscribe(const Online::Event::EventToken& token) noexcept 
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Event::EventDispatcher>()->UnSubscribe(token);
    }
    inline void OnEmit(const Online::Event::Event& e) noexcept 
    {
        Online::Runtime::Context::Instance().GetModule<Online::Event::EventDispatcher>()->Emit(e);
    }
#pragma endregion

#pragma region Task
    void OnPostJob(std::function<void()> func, std::string_view name) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Task::TaskScheduler>()->PostJob(std::move(func), name);
    }
#pragma endregion

#pragma region Net
    Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& OnGetMessageQueue() noexcept
    {
        return Online::Runtime::ServerContext::Instance().GetSerrverModule<Online::Net::Server::NetworkServer>()->GetMessageQueue();
    }
    bool OnSend(int connectionId, std::span<const std::byte> data) noexcept
    {
        return Online::Runtime::ServerContext::Instance().GetSerrverModule<Online::Net::Server::NetworkServer>()->Send(connectionId, data);
    }
    inline bool OnBroadcast(std::span<const std::byte> data) noexcept
    {
        return Online::Runtime::ServerContext::Instance().GetSerrverModule<Online::Net::Server::NetworkServer>()->Broadcast(data);
    }
#pragma endregion
}
int Online::Runtime::Server::Execute()
{
    if (!Initialize()) return -1;

    while (IsRunning())
    {
        BeginFrame();
        FixedUpdate();
        Update();
        LateUpdate();
        EndFrame();
        FrameSync();
    }

    Release();
    return 0;
}
void Online::Runtime::Server::Terminate()
{
    Release();
}
#define MODULE(T) modules.T
#define FUNCTABLE(T) funcTables.T
bool Online::Runtime::Server::Initialize()
{
    //Fill Context
    do
    {
#define FILL_COMMON_MODULE(T)     Online::Runtime::Context::Instance().commonModules.T = &MODULE(T)
#define FILL_COMMON_FUNCTABLE(T)  Online::Runtime::Context::Instance().commonFuncTables.T = &FUNCTABLE(T)

#define FILL_SERVER_MODULE(T)     Online::Runtime::ServerContext::Instance().serverModules.T = &MODULE(T)
#define FILL_SERVER_FUNCTABLE(T)  Online::Runtime::ServerContext::Instance().serverFuncTables.T = &FUNCTABLE(T)

        FILL_COMMON_MODULE(PhysicsSimulator);
        FILL_COMMON_MODULE(Chronometer);
        FILL_COMMON_MODULE(EventDispatcher);
        FILL_COMMON_MODULE(Logger);
        FILL_COMMON_MODULE(ThreadTracker);
        FILL_COMMON_MODULE(TaskScheduler);

        FILL_SERVER_MODULE(NetworkServer);

        FILL_COMMON_FUNCTABLE(EventDispatcher);
        FILL_COMMON_FUNCTABLE(Logger);
        FILL_COMMON_FUNCTABLE(ThreadTracker);
        FILL_COMMON_FUNCTABLE(Chronometer);
        FILL_COMMON_FUNCTABLE(TaskScheduler);

        FILL_SERVER_FUNCTABLE(NetworkServer);

#undef FILL_COMMON_MODULE
#undef FILL_COMMON_FUNCTABLE
    } while (false);

    //Fill FuncTables
    do
    {
#pragma region Time
        FUNCTABLE(Chronometer).Ondelta = Ondelta;
        FUNCTABLE(Chronometer).OnFixdelta = OnFixdelta;
        FUNCTABLE(Chronometer).OnUnscaledDelta = OnUnscaledDelta;
        FUNCTABLE(Chronometer).OnTimeScale = OnTimeScale;
        FUNCTABLE(Chronometer).OnFramerate = OnFramerate;
        FUNCTABLE(Chronometer).OnMilliseconds = OnMilliseconds;
        FUNCTABLE(Chronometer).OnSeconds = OnSeconds;
#pragma endregion

#pragma region Log
        FUNCTABLE(Logger).OnLog = OnLog;
#pragma endregion

#pragma region Thread
        FUNCTABLE(ThreadTracker).OnRegisterThread = OnRegisterThread;
        FUNCTABLE(ThreadTracker).OnUnregisterThread = OnUnregisterThread;
        FUNCTABLE(ThreadTracker).OnGetThreadName = OnGetThreadName;
        FUNCTABLE(ThreadTracker).OnGetThreadIsRunning = OnGetThreadIsRunning;
#pragma endregion

#pragma region Event
        FUNCTABLE(EventDispatcher).OnSubscribe = OnSubscribe;
        FUNCTABLE(EventDispatcher).OnUnSubscribe = OnUnSubscribe;
        FUNCTABLE(EventDispatcher).OnEmit = OnEmit;
#pragma endregion

#pragma region Task
        FUNCTABLE(TaskScheduler).OnPostJob = OnPostJob;
#pragma endregion

#pragma region Net
        FUNCTABLE(NetworkServer).OnGetMessageQueue = OnGetMessageQueue;
        FUNCTABLE(NetworkServer).OnSend = OnSend;
        FUNCTABLE(NetworkServer).OnBroadcast = OnBroadcast;
#pragma endregion

    } while (false);

    //Check FuncTables
    do
    {
        if (!FUNCTABLE(EventDispatcher).Check()) { throw std::runtime_error("Server [FuncTable] Miss [EventDispatcher]!"); }
        if (!FUNCTABLE(Logger).Check()) { throw std::runtime_error("Server [FuncTable] Miss [Logger]!"); }
        if (!FUNCTABLE(ThreadTracker).Check()) { throw std::runtime_error("Server [FuncTable] Miss [ThreadTracker]!"); }
        if (!FUNCTABLE(Chronometer).Check()) { throw std::runtime_error("Server [FuncTable] Miss [Chronometer]!"); }
        if (!FUNCTABLE(TaskScheduler).Check()) { throw std::runtime_error("Server [FuncTable] Miss [TaskScheduler]!"); }
        if (!FUNCTABLE(NetworkServer).Check()) { throw std::runtime_error("Server [FuncTable] Miss [NetworkServer]!"); }
    } while (false);

    //Create Modules
    do
    {
        if (!MODULE(Chronometer).Create()) { throw std::runtime_error("Server [Module] [Chronometer] Create Fail!"); }
        if (!MODULE(ThreadTracker).Create()) { throw std::runtime_error("Server [Module] [ThreadTracker] Create Fail!"); }
        if (!MODULE(EventDispatcher).Create()) { throw std::runtime_error("Server [Module] [EventDispatcher] Create Fail!"); }
        if (!MODULE(Logger).Create()) { throw std::runtime_error("Server [Module] [Logger] Create Fail!"); }
        if (!MODULE(PhysicsSimulator).Create(Server::PhysicAPI)) { throw std::runtime_error("Server [Module] [PhysicsSimulator] Create Fail!"); }
        if (!MODULE(TaskScheduler).Create()) { throw std::runtime_error("Server [Module] [TaskScheduler] Create Fail!"); }
        if (!MODULE(NetworkServer).Create()) { throw std::runtime_error("Server [Module] [NetworkServer] Create Fail!"); }
    } while (false);

    //Check Modules
    do
    {
        if (!MODULE(EventDispatcher)) { throw std::runtime_error("Server [Module] Miss [EventDispatcher]!"); }
        if (!MODULE(ThreadTracker)) { throw std::runtime_error("Server [Module] Miss [ThreadTracker]!"); }
        if (!MODULE(Logger)) { throw std::runtime_error("Server [Module] Miss [Logger]!"); }
        if (!MODULE(Chronometer)) { throw std::runtime_error("Server [Module] Miss [Chronometer]!"); }
        if (!MODULE(TaskScheduler)) { throw std::runtime_error("Server [Module] Miss [TaskScheduler]!"); }
        if (!MODULE(NetworkServer)) { throw std::runtime_error("Server [Module] Miss [NetworkServer]!"); }
    } while (false);

    //Initialize Modules
    do
    {
        if (!MODULE(Chronometer).Initialize()) { throw std::runtime_error("Server [Chronometer] Initialize Fail!"); }
        if (!MODULE(ThreadTracker).Initialize()) { throw std::runtime_error("Server [ThreadTracker] Initialize Fail!"); }
        if (!MODULE(Logger).Initialize()) { throw std::runtime_error("Server [Logger] Initialize Fail!"); }
        if (!MODULE(TaskScheduler).Initialize()) { throw std::runtime_error("Server [TaskScheduler] Initialize Fail!"); }
        if (!MODULE(EventDispatcher).Initialize()) { throw std::runtime_error("Server [EventDispatcher] Initialize Fail!"); }
        if (!MODULE(NetworkServer).Initialize()) { throw std::runtime_error("Server [NetworkServer] Initialize Fail!"); }
    } while (false);
    return true;
}
bool Online::Runtime::Server::IsRunning() const 
{ 
    return true;
}
void Online::Runtime::Server::BeginFrame() 
{
    MODULE(Chronometer)->Tick();
}
void Online::Runtime::Server::FixedUpdate() 
{
    MODULE(PhysicsSimulator).FixedUpdate();
}
void Online::Runtime::Server::Update() 
{ 
}
void Online::Runtime::Server::LateUpdate() 
{ 
}
void Online::Runtime::Server::EndFrame() 
{ 
}
void Online::Runtime::Server::FrameSync() 
{ 
    MODULE(Chronometer)->FrameSync();
}
void Online::Runtime::Server::Release()
{
    //Release Modules
    do
    {
        MODULE(NetworkServer).Release();
        MODULE(Chronometer).Release();
        MODULE(EventDispatcher).Release();
        MODULE(ThreadTracker).Release();
        MODULE(Logger).Release();
        MODULE(TaskScheduler).Release();
        MODULE(PhysicsSimulator).Release();
    } while (false);

    //Destroy Modules
    do
    {
        MODULE(NetworkServer).Destroy();
        MODULE(Chronometer).Destroy();
        MODULE(EventDispatcher).Destroy();
        MODULE(ThreadTracker).Destroy();
        MODULE(Logger).Destroy();
        MODULE(TaskScheduler).Destroy();
        MODULE(PhysicsSimulator).Destroy();
    } while (false);

    //UnRegister FuncTables
    do
    {
        FUNCTABLE(NetworkServer).UnRegister();
        FUNCTABLE(EventDispatcher).UnRegister();
        FUNCTABLE(Logger).UnRegister();
        FUNCTABLE(ThreadTracker).UnRegister();
        FUNCTABLE(Chronometer).UnRegister();
        FUNCTABLE(TaskScheduler).UnRegister();
    } while (false);

    //UnRegister Context
    Online::Runtime::Context::Instance().UnRegister();
}
#undef MODULE
#undef FUNCTABLE