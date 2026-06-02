#pragma once
#include <Client/Context/ClientContext.h>
#include <Context/Common/Module.h>

#include <Time/Common/FuncTable.h>
#include <Log/Common/FuncTable.h>
#include <Thread/Common/FuncTable.h>
#include <Event/Common/FuncTable.h>
#include <Input/Common/FuncTable.h>
#include <Window/Common/FuncTable.h>
#include <Config/Common/FuncTable.h>
#include <Asset/Common/FuncTable.h>
#include <Task/Common/FuncTable.h>
#include <Game/Common/FuncTable.h>
#include <Net/Client/Common/FuncTable.h>
#include <Phys/Common/FuncTable.h>
#include <Script/Common/FuncTable.h>
namespace Online::Runtime
{
    class Client final : public Online::Core::Singleton<Client>
    {
        friend class Online::Core::Singleton<Client>;

    private:
        struct Modules
        {
            Online::Runtime::Module<Online::Render::Renderer>           Renderer;
            Online::Runtime::Module<Online::Render::RenderPipeline>     RenderPipeline;
            Online::Runtime::Module<Online::Log::Logger>                Logger;
            Online::Runtime::Module<Online::Event::EventDispatcher>     EventDispatcher;
            Online::Runtime::Module<Online::Thread::ThreadTracker>      ThreadTracker;
            Online::Runtime::Module<Online::Time::Chronometer>          Chronometer;
            Online::Runtime::Module<Online::Input::InputMonitor>        InputMonitor;
            Online::Runtime::Module<Online::Window::Window>             Window;
            Online::Runtime::Module<Online::Physics::PhysicsSimulator>  PhysicsSimulator;
            Online::Runtime::Module<Online::Config::Configurator>       Configurator;
            Online::Runtime::Module<Online::Asset::AssetHub>            AssetHub;
            Online::Runtime::Module<Online::Task::TaskScheduler>        TaskScheduler;
            Online::Runtime::Module<Online::Game::GameWorld>            GameWorld;
            Online::Runtime::Module<Online::Net::Client::NetworkClient> NetworkClient;
            Online::Runtime::Module<Online::Audio::AudioPlayer>         AudioPlayer;
            Online::Runtime::Module<Online::Script::LifeCycleTable>     LifeCycleTable;
        };

        struct FuncTables
        {
            Online::Runtime::FuncTable<Online::Time::Chronometer>          Chronometer;
            Online::Runtime::FuncTable<Online::Log::Logger>                Logger;
            Online::Runtime::FuncTable<Online::Thread::ThreadTracker>      ThreadTracker;
            Online::Runtime::FuncTable<Online::Event::EventDispatcher>     EventDispatcher;
            Online::Runtime::FuncTable<Online::Config::Configurator>       Configurator;
            Online::Runtime::FuncTable<Online::Input::InputMonitor>        InputMonitor;
            Online::Runtime::FuncTable<Online::Window::Window>             Window;
            Online::Runtime::FuncTable<Online::Asset::AssetHub>            AssetHub;
            Online::Runtime::FuncTable<Online::Task::TaskScheduler>        TaskScheduler;
            Online::Runtime::FuncTable<Online::Game::GameWorld>            GameWorld;
            Online::Runtime::FuncTable<Online::Net::Client::NetworkClient> NetworkClient;
            Online::Runtime::FuncTable<Online::Physics::PhysicsSimulator>  PhysicsSimulator;
            Online::Runtime::FuncTable<Online::Script::LifeCycleTable>     LifeCycleTable;
        };

    private:
        Client() = default;
        ~Client() = default;

    public:
        Client(const Client&) = delete;
        Client& operator=(const Client&) = delete;
        Client(Client&&) = delete;
        Client& operator=(Client&&) = delete;

    public:
        int Execute();
        void Terminate();

    private:
        bool Initialize();
        bool IsRunning() const;
        void BeginFrame();
        void FixedUpdate();
        void Update();
        void LateUpdate();
        void Render();
        void EndFrame();
        void FrameSync();
        void Release();

    private:
        Modules modules;
        FuncTables funcTables;
    };
}

extern template class Online::Core::Singleton<Online::Runtime::Client>;
