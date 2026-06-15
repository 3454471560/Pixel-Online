#include <Server/Boostrap.h>
#include <Server/Server.h>
#include <Time/Chronometer.h>
#include <Log/Logger.h>
#include <Thread/ThreadTracker.h>
#include <Event/EventDispatcher.h>
#include <Phys/PhysicsSimulator.h>
#include <Task/TaskScheduler.h>
#include <Net/Server/HybridServer.h>
#include <Game/GameWorld.h>
#include <Script/LifeCycleTable.h>
#include <Config/Configurator.h>
#include<Input/InputMonitor.h>

#include<Game/Scene/Scene.h>
#include<Game/Entity/GameObject.h>

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

#pragma region Input
    inline bool OnIsClientKeyHold(uint32_t connId, Online::Input::KeyCode key) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->IsClientKeyHold(connId, key);
    }
    inline bool OnConsumeClientTrigger(uint32_t connId, Online::Input::KeyCode key) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->ConsumeClientTrigger(connId, key);
    }
#pragma endregion

#pragma region Config
    inline Online::Render::API OnGetRenderAPI() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Config::Configurator>()->GetRenderAPI();
    }
    inline bool OnGetEnableVSync() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Config::Configurator>()->GetEnableVSync();
    }
    inline const Online::TileEdit::TileMap& OnGetTileMap(Online::Config::TileMapID ID) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Config::Configurator>()->GetTileMap(ID);
    }
    inline const std::vector<Online::Config::CharLayout>& OnGetCharLayouts() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Config::Configurator>()->GetCharLayouts();
    }
#pragma endregion

#pragma region Task
    void OnPostJob(std::function<void()> func, std::string_view name) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Task::TaskScheduler>()->PostJob(std::move(func), name);
    }
#pragma endregion

#pragma region Game
    inline Online::Game::Scene* OnGetActiveScene() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetActiveScene();
    }
    inline entt::registry& OnGetRegistry() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetRegistry();
    }
    inline void OnDestroyEntity(entt::entity entity) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->DestroyEntity(entity);
    }
    inline Online::Game::GameObject* OnGetGameObject(entt::entity entity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetGameObject(entity);
    }
    inline void OnSetRelationship(entt::entity childId, entt::entity parentId, entt::entity afterSibling, bool keepWorldTransform) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->SetRelationship(childId, parentId, afterSibling, keepWorldTransform);
    }
    inline void OnTransformUpdater(entt::entity entity, glm::vec2 trans, float angle) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->TransformUpdater(entity, trans, angle);
    }
    inline glm::vec2 OnGetWorldPosition(entt::entity entity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetWorldPosition(entity);
    }
    inline float OnGetWorldRotation(entt::entity entity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetWorldRotation(entity);
    }
    inline void OnSwitchSceneAfterLoadingAsync(const std::string& newSceneName) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->SwitchSceneAfterLoadingAsync(newSceneName);
    }
    inline void OnSwitchSceneAsync(const std::string& newSceneName) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->SwitchSceneAsync(newSceneName);
    }
    inline void OnLoadScene(const std::string& sceneName) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->LoadScene(sceneName);
    }
    inline bool OnIsSceneLoading() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->IsSceneLoading();
    }
    inline void OnDisplayPendingScene() noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->DisplayPendingScene();
    }
    inline bool OnIsPendingSceneReady() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->IsPendingSceneReady();
    }
    inline Online::Game::GameObject* OnFindGameObjectByName(std::string_view name) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->FindGameObjectByName(name);
    }
    inline std::vector<Online::Game::GameObject*> OnFindGameObjectsAllByName(std::string_view name) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->FindGameObjectsAllByName(name);
    }
    inline Online::Game::GameObject* OnFindGameObjectByTag(std::string_view tagName) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->FindGameObjectByTag(tagName);
    }
    inline std::vector<Online::Game::GameObject*> OnFindGameObjectsByTag(std::string_view tagName) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->FindGameObjectsByTag(tagName);
    }
    inline uint32_t OnGenerate() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->Generate();
    }
    inline uint32_t OnGetServerFrame() noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetServerFrame();
	}
    inline void OnAddAnimatorControll(Online::Game::RoleID roleID, Online::Game::GameObject* player) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->AddAnimatorControll(roleID, player);
    }
#pragma endregion

#pragma region Net
    inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>&
        OnGetMessageQueue(Online::Net::PacketType type) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerModule<Online::Net::Server::HybridServer>()
            ->GetMessageQueue(type);
    }
    inline bool OnSendReliable(int connectionId,
        std::span<const std::byte> data,
        Online::Net::PacketType type,
        Online::Net::ChannelType channel) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerModule<Online::Net::Server::HybridServer>()
            ->SendReliable(connectionId, data, type, channel);
    }
    inline bool OnSendUnreliable(int connectionId,
        std::span<const std::byte> data,
        Online::Net::PacketType type,
        Online::Net::ChannelType channel) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerModule<Online::Net::Server::HybridServer>()
            ->SendUnreliable(connectionId, data, type, channel);
    }
    inline bool OnBroadcastReliable(std::span<const std::byte> data,
        Online::Net::PacketType type,
        Online::Net::ChannelType channel) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerModule<Online::Net::Server::HybridServer>()
            ->BroadcastReliable(data, type, channel);
    }
    inline bool OnBroadcastUnreliable(std::span<const std::byte> data,
        Online::Net::PacketType type,
        Online::Net::ChannelType channel) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerModule<Online::Net::Server::HybridServer>()
            ->BroadcastUnreliable(data, type, channel);
    }
    inline bool OnBroadcastUnreliableExcept(int excludeConnId,
        std::span<const std::byte> data,
        Online::Net::PacketType type,
        Online::Net::ChannelType channel = Online::Net::ChannelType::Unreliable) noexcept
    {
        return Online::Runtime::ServerContext::Instance()
            .GetServerModule<Online::Net::Server::HybridServer>()
            ->BroadcastUnreliableExcept(excludeConnId, data, type, channel);
    }
#pragma endregion

#pragma region Physics
    inline void OnRemoveBody(entt::entity entity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->RemoveBody(entity);
    }

    inline void OnSetGravity(glm::vec2 gravity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->SetGravity(gravity);
    }

    inline glm::vec2 OnGetLinearVelocity(entt::entity entity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->GetLinearVelocity(entity);
    }

    inline void OnSetLinearVelocity(entt::entity entity, const glm::vec2& velocity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->SetLinearVelocity(entity, velocity);
    }

    inline float OnGetAngularVelocity(entt::entity entity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->GetAngularVelocity(entity);
    }

    inline void OnSetAngularVelocity(entt::entity entity, float omega) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->SetAngularVelocity(entity, omega);
    }

    inline void OnApplyForce(entt::entity entity, const glm::vec2& force) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->ApplyForce(entity, force);
    }

    inline void OnApplyForceAtPoint(entt::entity entity, const glm::vec2& force, const glm::vec2& worldPoint) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->ApplyForceAtPoint(entity, force, worldPoint);
    }

    inline void OnApplyLinearImpulse(entt::entity entity, const glm::vec2& impulse) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->ApplyLinearImpulse(entity, impulse);
    }

    inline void OnApplyLinearImpulseAtPoint(entt::entity entity, const glm::vec2& impulse, const glm::vec2& worldPoint) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->ApplyLinearImpulseAtPoint(entity, impulse, worldPoint);
    }

    inline bool OnIsAwake(entt::entity entity) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->IsAwake(entity);
    }

    inline void OnSetAwake(entt::entity entity, bool awake) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->SetAwake(entity, awake);
    }

    inline void OnAddDebugRay(glm::vec2 origin, glm::vec2 direction, float length, glm::vec4 color) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->AddDebugRay(origin, direction, length, color);
    }

    inline bool OnRayCast(glm::vec2 origin, glm::vec2 angleRad, float maxDistance, Online::Physics::RayCastHit& outHit) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->RayCast(origin, angleRad, maxDistance, outHit);
    }

    inline bool OnRayCastLayer(glm::vec2 origin, glm::vec2 angleRad, float maxDistance, Online::Physics::RayCastHit& outHit, uint16_t layerMask, bool includeTriggers) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->RayCast(origin, angleRad, maxDistance, outHit, layerMask, includeTriggers);
    }

    inline void OnSetBodyTransform(entt::entity entity, const glm::vec2& position, float angle) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->SetBodyTransform(entity, position, angle);
    }
#pragma endregion

#pragma region Script
    inline void OnRegister(const Online::Script::ScriptFunctionInfo& info) noexcept
    {
        Online::Runtime::Context::Instance().GetModule<Online::Script::LifeCycleTable>()->Register(info);
    }
    inline const Online::Script::ScriptFunctionInfo* OnGetInfo(Online::Script::ScriptFunctionID id) noexcept
    {
        return Online::Runtime::Context::Instance().GetModule<Online::Script::LifeCycleTable>()->GetInfo(id);
    }
#pragma endregion
}
int Online::Runtime::Server::Execute()
{
    if (!Initialize()) return -1;

    while (IsRunning())
    {
        FixedUpdate();
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
		FILL_COMMON_MODULE(GameWorld);
        FILL_COMMON_MODULE(LifeCycleTable);
        FILL_COMMON_MODULE(Configurator);
        FILL_COMMON_MODULE(InputMonitor);

        FILL_SERVER_MODULE(NetworkServer);

        FILL_COMMON_FUNCTABLE(PhysicsSimulator);
        FILL_COMMON_FUNCTABLE(EventDispatcher);
        FILL_COMMON_FUNCTABLE(Logger);
        FILL_COMMON_FUNCTABLE(ThreadTracker);
        FILL_COMMON_FUNCTABLE(Chronometer);
        FILL_COMMON_FUNCTABLE(TaskScheduler);
		FILL_COMMON_FUNCTABLE(GameWorld);
        FILL_COMMON_FUNCTABLE(LifeCycleTable);
        FILL_COMMON_FUNCTABLE(Configurator);
        FILL_COMMON_FUNCTABLE(InputMonitor);

        FILL_SERVER_FUNCTABLE(NetworkServer);

#undef FILL_COMMON_MODULE
#undef FILL_COMMON_FUNCTABLE
    } while (false);

    //Fill FuncTables
    do
    {
#pragma region Time
        FUNCTABLE(Chronometer).OnDelta = Ondelta;
        FUNCTABLE(Chronometer).OnFixDelta = OnFixdelta;
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

#pragma region Input
        FUNCTABLE(InputMonitor).OnIsClientKeyHold = OnIsClientKeyHold;
        FUNCTABLE(InputMonitor).OnConsumeClientTrigger = OnConsumeClientTrigger;
#pragma endregion

#pragma region Config
        FUNCTABLE(Configurator).OnGetRenderAPI = OnGetRenderAPI;
        FUNCTABLE(Configurator).OnGetEnableVSync = OnGetEnableVSync;
        FUNCTABLE(Configurator).GetTileMap = OnGetTileMap;
        FUNCTABLE(Configurator).GetCharLayouts = OnGetCharLayouts;
#pragma endregion

#pragma region Task
        FUNCTABLE(TaskScheduler).OnPostJob = OnPostJob;
#pragma endregion

#pragma region Game
        FUNCTABLE(GameWorld).OnGetActiveScene = OnGetActiveScene;
        FUNCTABLE(GameWorld).OnGetRegistry = OnGetRegistry;
        FUNCTABLE(GameWorld).OnDestroyEntity = OnDestroyEntity;
        FUNCTABLE(GameWorld).OnGetGameObject = OnGetGameObject;
        FUNCTABLE(GameWorld).OnSetRelationship = OnSetRelationship;
        FUNCTABLE(GameWorld).OnTransformUpdater = OnTransformUpdater;
        FUNCTABLE(GameWorld).OnGetWorldPosition = OnGetWorldPosition;
        FUNCTABLE(GameWorld).OnGetWorldRotation = OnGetWorldRotation;
        FUNCTABLE(GameWorld).OnSwitchSceneAfterLoadingAsync = OnSwitchSceneAfterLoadingAsync;
        FUNCTABLE(GameWorld).OnSwitchSceneAsync = OnSwitchSceneAsync;
        FUNCTABLE(GameWorld).OnLoadScene = OnLoadScene;
        FUNCTABLE(GameWorld).OnIsSceneLoading = OnIsSceneLoading;
        FUNCTABLE(GameWorld).OnDisplayPendingScene = OnDisplayPendingScene;
        FUNCTABLE(GameWorld).OnIsPendingSceneReady = OnIsPendingSceneReady;

        FUNCTABLE(GameWorld).OnFindGameObjectByName = OnFindGameObjectByName;
        FUNCTABLE(GameWorld).OnFindGameObjectsAllByName = OnFindGameObjectsAllByName;
        FUNCTABLE(GameWorld).OnFindGameObjectByTag = OnFindGameObjectByTag;
        FUNCTABLE(GameWorld).OnFindGameObjectsByTag = OnFindGameObjectsByTag;
        FUNCTABLE(GameWorld).OnGenerate = OnGenerate;
		FUNCTABLE(GameWorld).OnGetServerFrame = OnGetServerFrame;
        FUNCTABLE(GameWorld).OnAddAnimatorControll = OnAddAnimatorControll;
#pragma endregion

#pragma region Net
        FUNCTABLE(NetworkServer).OnGetMessageQueue = OnGetMessageQueue;
        FUNCTABLE(NetworkServer).OnSendReliable = OnSendReliable;
        FUNCTABLE(NetworkServer).OnSendUnreliable = OnSendUnreliable;
        FUNCTABLE(NetworkServer).OnBroadcastReliable = OnBroadcastReliable;
        FUNCTABLE(NetworkServer).OnBroadcastUnreliable = OnBroadcastUnreliable;
        FUNCTABLE(NetworkServer).OnBroadcastUnreliableExcept = OnBroadcastUnreliableExcept;
#pragma endregion

#pragma region Physics
        FUNCTABLE(PhysicsSimulator).OnRemoveBody = OnRemoveBody;
        FUNCTABLE(PhysicsSimulator).OnSetGravity = OnSetGravity;
        FUNCTABLE(PhysicsSimulator).OnGetLinearVelocity = OnGetLinearVelocity;
        FUNCTABLE(PhysicsSimulator).OnSetLinearVelocity = OnSetLinearVelocity;
        FUNCTABLE(PhysicsSimulator).OnGetAngularVelocity = OnGetAngularVelocity;
        FUNCTABLE(PhysicsSimulator).OnSetAngularVelocity = OnSetAngularVelocity;
        FUNCTABLE(PhysicsSimulator).OnApplyForce = OnApplyForce;
        FUNCTABLE(PhysicsSimulator).OnApplyForceAtPoint = OnApplyForceAtPoint;
        FUNCTABLE(PhysicsSimulator).OnApplyLinearImpulse = OnApplyLinearImpulse;
        FUNCTABLE(PhysicsSimulator).OnApplyLinearImpulseAtPoint = OnApplyLinearImpulseAtPoint;
        FUNCTABLE(PhysicsSimulator).OnIsAwake = OnIsAwake;
        FUNCTABLE(PhysicsSimulator).OnSetAwake = OnSetAwake;
        FUNCTABLE(PhysicsSimulator).OnAddDebugRay = OnAddDebugRay;
        FUNCTABLE(PhysicsSimulator).OnRayCast = OnRayCast;
        FUNCTABLE(PhysicsSimulator).OnRayCastLayer = OnRayCastLayer;
        FUNCTABLE(PhysicsSimulator).OnSetBodyTransform = OnSetBodyTransform;
#pragma endregion

#pragma region Script
        FUNCTABLE(LifeCycleTable).OnRegister = OnRegister;
        FUNCTABLE(LifeCycleTable).OnGetInfo = OnGetInfo;
#pragma endregion


    } while (false);

    //Check FuncTables
    do
    {
        if (!FUNCTABLE(Configurator).Check()) { throw std::runtime_error("Server [FuncTable] Miss [Configurator]!"); }
        if (!FUNCTABLE(EventDispatcher).Check()) { throw std::runtime_error("Server [FuncTable] Miss [EventDispatcher]!"); }
        if (!FUNCTABLE(Logger).Check()) { throw std::runtime_error("Server [FuncTable] Miss [Logger]!"); }
        if (!FUNCTABLE(ThreadTracker).Check()) { throw std::runtime_error("Server [FuncTable] Miss [ThreadTracker]!"); }
        if (!FUNCTABLE(Chronometer).Check()) { throw std::runtime_error("Server [FuncTable] Miss [Chronometer]!"); }
        if (!FUNCTABLE(TaskScheduler).Check()) { throw std::runtime_error("Server [FuncTable] Miss [TaskScheduler]!"); }
        if (!FUNCTABLE(GameWorld).Check()) { throw std::runtime_error("Online [FuncTable] Miss [GameWorld]!"); }
        if (!FUNCTABLE(NetworkServer).Check()) { throw std::runtime_error("Server [FuncTable] Miss [NetworkServer]!"); }
        if (!FUNCTABLE(PhysicsSimulator).Check()) { throw std::runtime_error("Server [FuncTable] Miss [PhysicsSimulator]!"); }
        if (!FUNCTABLE(LifeCycleTable).Check()) { throw std::runtime_error("Server [FuncTable] Miss [LifeCycleTable]!"); }
        if (!FUNCTABLE(InputMonitor).Check()) { throw std::runtime_error("Server [FuncTable] Miss [InputMonitor]!"); }
    } while (false);

    //Create Modules
    do
    {
        if (!MODULE(Configurator).Create()) { throw std::runtime_error("Server [Module] [Configurator] Create Fail!"); }
        if (!MODULE(Chronometer).Create()) { throw std::runtime_error("Server [Module] [Chronometer] Create Fail!"); }
        if (!MODULE(InputMonitor).Create()) { throw std::runtime_error("Server [Module] [InputMonitor] Create Fail!"); }
        if (!MODULE(ThreadTracker).Create()) { throw std::runtime_error("Server [Module] [ThreadTracker] Create Fail!"); }
        if (!MODULE(EventDispatcher).Create()) { throw std::runtime_error("Server [Module] [EventDispatcher] Create Fail!"); }
        if (!MODULE(Logger).Create()) { throw std::runtime_error("Server [Module] [Logger] Create Fail!"); }
        if (!MODULE(PhysicsSimulator).Create()) { throw std::runtime_error("Server [Module] [PhysicsSimulator] Create Fail!"); }
        if (!MODULE(TaskScheduler).Create()) { throw std::runtime_error("Server [Module] [TaskScheduler] Create Fail!"); }
        if (!MODULE(GameWorld).Create()) { throw std::runtime_error("Online [Module] [GameWorld] Create Fail!"); }
        if (!MODULE(NetworkServer).Create()) { throw std::runtime_error("Server [Module] [NetworkServer] Create Fail!"); }
        if (!MODULE(LifeCycleTable).Create()) { throw std::runtime_error("Server [Module] [LifeCycleTable] Create Fail!"); }
    } while (false);

    //Check Modules
    do
    {
        if (!MODULE(Configurator)) { throw std::runtime_error("Server [Module] Miss [Configurator]!"); }
        if (!MODULE(InputMonitor)) { throw std::runtime_error("Server [Module] Miss [InputMonitor]!"); }
        if (!MODULE(EventDispatcher)) { throw std::runtime_error("Server [Module] Miss [EventDispatcher]!"); }
        if (!MODULE(ThreadTracker)) { throw std::runtime_error("Server [Module] Miss [ThreadTracker]!"); }
        if (!MODULE(Logger)) { throw std::runtime_error("Server [Module] Miss [Logger]!"); }
        if (!MODULE(Chronometer)) { throw std::runtime_error("Server [Module] Miss [Chronometer]!"); }
        if (!MODULE(TaskScheduler)) { throw std::runtime_error("Server [Module] Miss [TaskScheduler]!"); }
        if (!MODULE(GameWorld)) { throw std::runtime_error("Online [Module] Miss [GameWorld]!"); }
        if (!MODULE(NetworkServer)) { throw std::runtime_error("Server [Module] Miss [NetworkServer]!"); }
        if (!MODULE(PhysicsSimulator)) { throw std::runtime_error("Server [Module] Miss [PhysicsSimulator]!"); }
        if (!MODULE(LifeCycleTable)) { throw std::runtime_error("Server [Module] Miss [LifeCycleTable]!"); }
    } while (false);

    //Initialize Modules
    do
    {
        if (!MODULE(Configurator).Initialize()) { throw std::runtime_error("Server [Configurator] Initialize Fail!"); }
        if (!MODULE(Chronometer).Initialize()) { throw std::runtime_error("Server [Chronometer] Initialize Fail!"); }
        if (!MODULE(ThreadTracker).Initialize()) { throw std::runtime_error("Server [ThreadTracker] Initialize Fail!"); }
        if (!MODULE(Logger).Initialize()) { throw std::runtime_error("Server [Logger] Initialize Fail!"); }
        if (!MODULE(TaskScheduler).Initialize()) { throw std::runtime_error("Server [TaskScheduler] Initialize Fail!"); }
        if (!MODULE(EventDispatcher).Initialize()) { throw std::runtime_error("Server [EventDispatcher] Initialize Fail!"); }
        if (!MODULE(InputMonitor).Initialize()) { throw std::runtime_error("Server [InputMonitor] Initialize Fail!"); }
        if (!MODULE(GameWorld).Initialize()) { throw std::runtime_error("Online [GameWorld] Module Initialize Fail!"); }
        if (!MODULE(NetworkServer).Initialize()) { throw std::runtime_error("Server [NetworkServer] Initialize Fail!"); }
        if (!MODULE(PhysicsSimulator).Initialize()) { throw std::runtime_error("Server [PhysicsSimulator] Initialize Fail!"); }
        if (!MODULE(LifeCycleTable).Initialize()) { throw std::runtime_error("Server [LifeCycleTable] Initialize Fail!"); }
    } while (false);

    return true;
}
bool Online::Runtime::Server::IsRunning() const 
{ 
    return true;
}
void Online::Runtime::Server::FixedUpdate() 
{
    
    MODULE(Chronometer)->Tick();
	MODULE(InputMonitor).FixedUpdate();
	MODULE(GameWorld).Update();
    MODULE(GameWorld).FixedUpdate();

    auto* scene = MODULE(GameWorld)->GetActiveScene();
    if (scene)
    {
        for (auto [entity, transform, rb] : scene->GetView<Game::Transform, Game::Rigidbody>().each())
        {
            Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
            if (!obj || !obj->IsActive())
            {
                continue;
            }

            //if (!rb.IsVisible())
            //{
            //	continue;
            //}

            MODULE(PhysicsSimulator)->SubmitBodyCreation(
                entity,
                static_cast<b2BodyType>(rb.GetBodyType()),
                transform.GetWorldPosition(),
                transform.GetWorldRotation(),
                rb.IsFixRotation(),
                rb.getGravityScale(),
                rb.GetLinearDamping(),
                rb.GetAngularDamping(),
                rb.IsAwake()
            );
        }

        for (auto [entity, transform, col] : scene->GetView<Game::Transform, Game::Collider>().each())
        {
            Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
            if (!obj || !obj->IsActive())
            {
                continue;
            }

            MODULE(PhysicsSimulator)->SubmitColliderDesc(
                col.GetRigidEntity() != entt::null ? col.GetRigidEntity() : entity,
                col.GetShape(),
                col.GetOffset() * transform.GetWorldScale(),
                col.GetRadius() * ((transform.GetWorldScale().x + transform.GetWorldScale().y) * 0.5f),
                col.GetHalfSize() * transform.GetWorldScale(),
                col.GetAngle(),
                col.GetDensity(),
                col.GetFriction(),
                col.GetRestitution(),
                col.IsTrigger(),
                col.GetCategoryBits(),
                col.GetMaskBits(),
                col.GetGroupIndex()
            );
        }

        for (auto [entity, transform, collist] : scene->GetView<Game::Transform, Game::ColliderList>().each())
        {
            Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
            if (!obj || !obj->IsActive())
            {
                continue;
            }


            for (auto& col : collist)
            {
                MODULE(PhysicsSimulator)->SubmitColliderDesc(
                    col.GetRigidEntity(),
                    col.GetShape(),
                    { col.GetOffset().x * transform.GetWorldScale().x ,col.GetOffset().y * transform.GetWorldScale().y },
                    { col.GetRadius() * (transform.GetWorldScale().x + transform.GetWorldScale().y) * 0.5f },
                    { col.GetHalfSize().x * transform.GetWorldScale().x ,col.GetHalfSize().y * transform.GetWorldScale().y },
                    col.GetAngle(),
                    col.GetDensity(),
                    col.GetFriction(),
                    col.GetRestitution(),
                    col.IsTrigger(),
                    col.GetCategoryBits(),
                    col.GetMaskBits(),
                    col.GetGroupIndex()
                );
            }
        }

        for (auto [entity, transform, tilemap] : scene->GetView<Game::Transform, Game::TileMap>().each())
        {
            MODULE(PhysicsSimulator)->SubmitBodyCreation(entity,
                static_cast<b2BodyType>(tilemap.GetBodyType()),
                transform.GetWorldPosition(),
                transform.GetWorldRotation(),
                tilemap.IsFixedRotation(),
                tilemap.GetGravityScale(),
                tilemap.GetLinearDamping(),
                tilemap.GetAngularDamping(),
                tilemap.IsAwake());

            for (auto& col : tilemap)
            {
                MODULE(PhysicsSimulator)->SubmitColliderDesc(entity,
                    col.GetShape(),
                    col.GetOffset() * transform.GetWorldScale(),
                    col.GetRadius() * ((transform.GetWorldScale().x + transform.GetWorldScale().y) * 0.5f),
                    col.GetHalfSize() * transform.GetWorldScale(),
                    col.GetAngle(),
                    col.GetDensity(),
                    col.GetFriction(),
                    col.GetRestitution(),
                    col.IsTrigger(),
                    col.GetCategoryBits(),
                    col.GetMaskBits(),
                    col.GetGroupIndex());
            }
        }

    }
    MODULE(PhysicsSimulator)->SyncBodies();

    
    MODULE(PhysicsSimulator).FixedUpdate();
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
        MODULE(InputMonitor).Release();
        MODULE(EventDispatcher).Release();
        MODULE(ThreadTracker).Release();
        MODULE(Logger).Release();
        MODULE(TaskScheduler).Release();
        MODULE(PhysicsSimulator).Release();
        MODULE(Configurator).Release();
    } while (false);

    //Destroy Modules
    do
    {
        MODULE(NetworkServer).Destroy();
        MODULE(Chronometer).Destroy();
        MODULE(InputMonitor).Destroy();
        MODULE(EventDispatcher).Destroy();
        MODULE(ThreadTracker).Destroy();
        MODULE(Logger).Destroy();
        MODULE(TaskScheduler).Destroy();
        MODULE(PhysicsSimulator).Destroy();
        MODULE(Configurator).Destroy();
    } while (false);

    //UnRegister FuncTables
    do
    {
        FUNCTABLE(NetworkServer).UnRegister();
        FUNCTABLE(EventDispatcher).UnRegister();
        FUNCTABLE(InputMonitor).UnRegister();
        FUNCTABLE(Logger).UnRegister();
        FUNCTABLE(ThreadTracker).UnRegister();
        FUNCTABLE(Chronometer).UnRegister();
        FUNCTABLE(TaskScheduler).UnRegister();
        FUNCTABLE(Configurator).UnRegister();
    } while (false);

    //UnRegister Context
    Online::Runtime::Context::Instance().UnRegister();
}
#undef MODULE
#undef FUNCTABLE