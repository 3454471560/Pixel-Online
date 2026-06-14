#pragma once
#include <Server/Context/ServerContext.h>
#include <Context/Common/Module.h>

#include <Time/Common/FuncTable.h>
#include <Log/Common/FuncTable.h>
#include <Thread/Common/FuncTable.h>
#include <Event/Common/FuncTable.h>
#include <Task/Common/FuncTable.h>
#include <Net/Server/Common/FuncTable.h>
#include <Game/Common/FuncTable.h>
#include <Phys/Common/FuncTable.h>
#include <Script/Common/FuncTable.h>
#include <Config/Common/FuncTable.h>
#include <Input/Common/FuncTable.h>

namespace Online::Runtime
{
    class Server final : public Online::Core::Singleton<Server>
    {
        friend class Online::Core::Singleton<Server>;

        struct Modules
        {
            Module<Online::Log::Logger> Logger;
            Module<Online::Event::EventDispatcher> EventDispatcher;
            Module<Online::Thread::ThreadTracker> ThreadTracker;
            Module<Online::Time::Chronometer> Chronometer;
			Module<Online::Physics::PhysicsSimulator> PhysicsSimulator;
			Module<Online::Task::TaskScheduler> TaskScheduler;
            Module<Online::Net::Server::HybridServer> NetworkServer;
			Module<Online::Game::GameWorld> GameWorld;
            Module<Online::Script::LifeCycleTable> LifeCycleTable;
            Module<Online::Config::Configurator> Configurator;
            Module<Online::Input::InputMonitor> InputMonitor;
        };

        struct FuncTables
        {
            Online::Runtime::FuncTable<Online::Time::Chronometer> Chronometer;
            Online::Runtime::FuncTable<Online::Log::Logger> Logger;
            Online::Runtime::FuncTable<Online::Thread::ThreadTracker> ThreadTracker;
            Online::Runtime::FuncTable<Online::Event::EventDispatcher> EventDispatcher;
            Online::Runtime::FuncTable<Online::Task::TaskScheduler> TaskScheduler;
            Online::Runtime::FuncTable<Online::Net::Server::HybridServer> NetworkServer;
			Online::Runtime::FuncTable<Online::Game::GameWorld> GameWorld;
            Online::Runtime::FuncTable<Online::Physics::PhysicsSimulator> PhysicsSimulator;
            Online::Runtime::FuncTable<Online::Script::LifeCycleTable> LifeCycleTable;
            Online::Runtime::FuncTable<Online::Config::Configurator> Configurator;
            Online::Runtime::FuncTable<Online::Input::InputMonitor> InputMonitor;
        };

        Modules modules;
        FuncTables funcTables;


        Server() = default;
        ~Server() = default;

    public:
        int Execute();
        void Terminate();

    private:
        bool Initialize();
        bool IsRunning() const;
        void FixedUpdate();
        void EndFrame();
        void FrameSync();
        void Release();
    };
}

extern template class Online::Core::Singleton<Online::Runtime::Server>;
