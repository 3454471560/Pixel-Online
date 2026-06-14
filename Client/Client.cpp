#include<Client/Boostrap.h>
#include<Client/Client.h>
#include<Time/Chronometer.h>
#include<Log/Logger.h>
#include<Thread/ThreadTracker.h>
#include<Event/EventDispatcher.h>
#include<Input/InputMonitor.h>
#include<Window/Frontend/Window.h>
#include<Render/RenderPipeline.h>
#include<Render/Frontend/Renderer.h>
#include<Config/Configurator.h>
#include<Phys/PhysicsSimulator.h>
#include<Asset/AssetHub.h>
#include<Task/TaskScheduler.h>
#include<Game/GameWorld.h>
#include<Net/Client/HybridClient.h>
#include<Audio/AudioPlayer.h>
#include<Script/LifeCycleTable.h>
#include<Game/Scene/Scene.h>
#include<Game/Entity/GameObject.h>

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
	inline Online::Core::Thread::Identifier OnRegisterThread(std::string_view name, void(*Thread)(void*, void*), void* bootstraper, void* args)
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Thread::ThreadTracker>()->RegisterThread(name, Thread, bootstraper, args);
	}
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
#pragma endregion

#pragma region Event
	inline Online::Event::EventToken OnSubscribe(Online::Event::EventType type, void (*OnCallBack)(void*, const ::Online::Event::Event&), void* listener) noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Event::EventDispatcher>()->Subscribe(type, OnCallBack, listener);
	}
	inline bool OnUnSubscribe(const Online::Event::EventToken& eventToken) noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Event::EventDispatcher>()->UnSubscribe(eventToken);
	}
	inline void OnEmit(const Online::Event::Event& event) noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Event::EventDispatcher>()->Emit(event);
	}
#pragma endregion

#pragma region Input
	inline bool OnGetKeyDown(Online::Input::KeyCode keyCode) noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->GetKeyDown(keyCode);
	}
	inline bool OnGetKeyPressed(Online::Input::KeyCode keyCode) noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->GetKeyPressed(keyCode);
	}
	inline bool OnGetKeyReleased(Online::Input::KeyCode keyCode) noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->GetKeyReleased(keyCode);
	}
	inline void OnResetAllState() noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->ResetAllState();
	}
	inline void OnResetMouseState() noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->ResetMouseState();
	}
	inline glm::vec2 OnGetMousePosition() noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->GetMousePosition();

	}
	inline std::string OnGetTextInputBuffer() noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->GetTextInputBuffer();
	}
	inline void OnStartTextInput() noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->StartTextInput();
	}
	inline void OnStopTextInput() noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->StopTextInput();
	}
	inline std::string OnGetCompositionText() noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->GetCompositionText();
	}
	inline int OnGetCompositionCursor() noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->GetCompositionCursor();
	}
	inline void OnSetTextInputRect(int x, int y, int w, int h) noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Input::InputMonitor>()->SetTextInputRect(x, y, w, h);
	}
#pragma endregion

#pragma region Task
	void OnPostJob(std::function<void()> func, std::string_view name) noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Task::TaskScheduler>()->PostJob(std::move(func), name);
	}
#pragma endregion

#pragma region Asset
	inline void OnSetRenderer(SDL_Renderer* render) noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->SetRenderer(render);
	}
	inline void OnSyncLoadedAssets() noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->SyncLoadedAssets();
	}
	inline void OnInitOffScreen() noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->InitOffScreen();
	}
	inline void OnSaveTextureToPNG(SDL_Texture* texture) noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->SaveTextureToPNG(texture);
	}
	inline void OnSaveScreeneToPNG(const std::filesystem::path& path) noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->SaveWindowScreenshot(path);
	}
	inline SDL_Texture* OnGetTexture(Online::Asset::TextureID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetTexture(id);
	}
	inline Mix_Chunk* OnGetSound(Online::Asset::SoundID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetSound(id);
	}
	inline Mix_Music* OnGetMusic(Online::Asset::MusicID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetMusic(id);
	}
	inline TTF_Font* OnGetFont(Online::Asset::FontID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetFont(id);
	}
	inline Online::Asset::AnimationClip* OnGetAnim(Online::Asset::AnimationClipID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetAnim(id);
	}
	inline bool OnIsTextureReady(Online::Asset::TextureID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->IsTextureReady(id);
	}
	inline bool OnIsSoundReady(Online::Asset::SoundID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->IsSoundReady(id);
	}
	inline bool OnIsMusicReady(Online::Asset::MusicID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->IsMusicReady(id);
	}
	inline bool OnIsFontReady(Online::Asset::FontID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->IsFontReady(id);
	}
	inline bool OnIsAnimReady(Online::Asset::AnimationClipID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->IsAnimReady(id);
	}
	inline glm::ivec2 OnGetTextureSize(Online::Asset::TextureID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetTextureSize(id);
	}
	inline int OnGetFontSize(Online::Asset::FontID id) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetFontSize(id);
	}
	inline float OnGetAssetLoadProgress() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetAsstetLoadProgress();
	}
	inline glm::ivec2 OnGetFontAtlasCoord(char32_t ch) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetFontAtlasCoord(ch);
	}
	inline SDL_Rect OnGetFontAtlasSrcRect(Online::Asset::FontID id, char32_t ch) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetFontAtlasSrcRect(id, ch);
	}
	inline int GetFontAtlasAdvance(Online::Asset::FontID id, char32_t ch) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Asset::AssetHub>()->GetFontAtlasAdvance(id, ch);
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

#pragma region Window
	inline Online::Window::Window* OnGetWindow() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Window::Window>().Get();
	}

	inline void OnPollEvents()
	{
		Online::Runtime::ClientContext::Instance().GetClientModule<Online::Window::Window>()->PollEvents();
	}

	inline void OnCloseWindow()
	{
		Online::Runtime::ClientContext::Instance().GetClientModule<Online::Window::Window>()->CloseWindow();
	}

	inline bool OnIsClose() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Window::Window>()->IsClose();
	}

	inline void* OnGetNativeWindow() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientModule<Online::Window::Window>()->GetNativeWindow();
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
	inline void OnSendJoinWorldRequest(const std::string& playerName, uint64_t playerId) noexcept
	{
		Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->SendJoinWorldRequest(playerName, playerId);
	}
	inline uint32_t OnGetLocalPlayerNetId() noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetLocalPlayerNetId();
	}
	inline Online::Game::GameObject* OnGetLocalPlayer() noexcept
	{
		return Online::Runtime::Context::Instance().GetModule<Online::Game::GameWorld>()->GetLocalPlayer();
	}

#pragma endregion

#pragma region Net
	// 获取指定类型的消息队列
	inline Online::Core::ThreadSafeQueue<Online::Net::NetMessage>& OnGetMessageQueue(Online::Net::PacketType type) noexcept
	{
		return Online::Runtime::ClientContext::Instance()
			.GetClientModule<Online::Net::Client::HybridClient>()
			->GetMessageQueue(type);
	}

	// 可靠发送，支持通道选择（默认值由调用方控制，这里直接转发）
	inline bool OnSendReliable(std::span<const std::byte> data, Online::Net::PacketType type, Online::Net::ChannelType channel) noexcept
	{
		return Online::Runtime::ClientContext::Instance()
			.GetClientModule<Online::Net::Client::HybridClient>()
			->SendReliable(data, type, channel);
	}

	// 不可靠发送，支持通道选择
	inline bool OnSendUnreliable(std::span<const std::byte> data, Online::Net::PacketType type, Online::Net::ChannelType channel) noexcept
	{
		return Online::Runtime::ClientContext::Instance()
			.GetClientModule<Online::Net::Client::HybridClient>()
			->SendUnreliable(data, type, channel);
	}

	inline bool OnConnect(const std::string& ip, uint16_t port) noexcept
	{
		return Online::Runtime::ClientContext::Instance()
			.GetClientModule<Online::Net::Client::HybridClient>()
			->Connect(ip, port);
	}

	inline void OnDisconnect() noexcept
	{
		Online::Runtime::ClientContext::Instance()
			.GetClientModule<Online::Net::Client::HybridClient>()
			->Disconnect();
	}

	inline bool OnIsConnected() noexcept
	{
		return Online::Runtime::ClientContext::Instance()
			.GetClientModule<Online::Net::Client::HybridClient>()
			->IsConnected();
	}

	inline int OnGetLocalConnId() noexcept
	{
		return Online::Runtime::ClientContext::Instance()
			.GetClientModule<Online::Net::Client::HybridClient>()
			->GetLocalConnId();
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
		return Online::Runtime::Context::Instance().GetModule<Online::Physics::PhysicsSimulator>()->RayCast(origin, angleRad, maxDistance, outHit,layerMask,includeTriggers);
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
int Online::Runtime::Client::Execute()
{
	if (!Initialize())
	{
		return -1;
	}
	while (IsRunning())
	{
		BeginFrame();
		FixedUpdate();
		Update();
		LateUpdate();
		Render();
		EndFrame();
		FrameSync();
	}

	Release();
	return 0;
}
void Online::Runtime::Client::Terminate()
{
	Release();
}
#define MODULE(T) modules.T
#define FUNCTABLE(T) funcTables.T
bool Online::Runtime::Client::Initialize()
{
	//Fill Context
	do
	{
#define FILL_COMMON_MODULE(T)     Online::Runtime::Context::Instance().commonModules.T = &MODULE(T)
#define FILL_COMMON_FUNCTABLE(T)  Online::Runtime::Context::Instance().commonFuncTables.T = &FUNCTABLE(T)

#define FILL_CLIENT_MODULE(T)     Online::Runtime::ClientContext::Instance().clientModules.T = &MODULE(T)
#define FILL_CLIENT_FUNCTABLE(T)  Online::Runtime::ClientContext::Instance().clientFuncTables.T = &FUNCTABLE(T)


		FILL_COMMON_MODULE(PhysicsSimulator);	
		FILL_COMMON_MODULE(Chronometer);
		FILL_COMMON_MODULE(EventDispatcher);
		FILL_COMMON_MODULE(Logger);	
		FILL_COMMON_MODULE(ThreadTracker);
		FILL_COMMON_MODULE(TaskScheduler);
		FILL_COMMON_MODULE(LifeCycleTable);
		FILL_COMMON_MODULE(GameWorld);
		FILL_COMMON_MODULE(Configurator);
		FILL_COMMON_MODULE(InputMonitor);


		FILL_COMMON_FUNCTABLE(EventDispatcher);
		FILL_COMMON_FUNCTABLE(Logger);
		FILL_COMMON_FUNCTABLE(ThreadTracker);
		FILL_COMMON_FUNCTABLE(Chronometer);
		FILL_COMMON_FUNCTABLE(TaskScheduler);
		FILL_COMMON_FUNCTABLE(PhysicsSimulator);
		FILL_COMMON_FUNCTABLE(GameWorld);
		FILL_COMMON_FUNCTABLE(LifeCycleTable);
		FILL_COMMON_FUNCTABLE(Configurator);
		FILL_COMMON_FUNCTABLE(InputMonitor);

		FILL_CLIENT_MODULE(Renderer);
		FILL_CLIENT_MODULE(RenderPipeline);
		FILL_CLIENT_MODULE(AssetHub);
		FILL_CLIENT_MODULE(Window);
		FILL_CLIENT_MODULE(NetworkClient);
		FILL_CLIENT_MODULE(AudioPlayer);

		FILL_CLIENT_FUNCTABLE(AssetHub);
		FILL_CLIENT_FUNCTABLE(Window);
		FILL_CLIENT_FUNCTABLE(NetworkClient);

#undef FILL_COMMON_MODULE
#undef FILL_COMMON_FUNCTABLE
#undef FILL_CLIENT_MODULE
#undef FILL_CLIENT_FUNCTABLE
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

#pragma region Window
		auto& window = funcTables.Window;
		window.OnGetWindow = OnGetWindow;
		window.OnPollEvents = OnPollEvents;
		window.OnCloseWindow = OnCloseWindow;
		window.OnIsClose = OnIsClose;
		window.OnGetNativeWindow = OnGetNativeWindow;
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
		FUNCTABLE(EventDispatcher).OnUnSubscribe = OnUnSubscribe;;
		FUNCTABLE(EventDispatcher).OnEmit = OnEmit;
#pragma endregion

#pragma region Asset
		FUNCTABLE(AssetHub).OnSetRenderer = OnSetRenderer;
		FUNCTABLE(AssetHub).OnSyncLoadedAssets = OnSyncLoadedAssets;
		FUNCTABLE(AssetHub).OnInitOffScreen = OnInitOffScreen;
		FUNCTABLE(AssetHub).OnSaveTextureToPNG = OnSaveTextureToPNG;
		FUNCTABLE(AssetHub).OnSaveScreenToPNG = OnSaveScreeneToPNG;
		FUNCTABLE(AssetHub).OnIsTextureReady = OnIsTextureReady;
		FUNCTABLE(AssetHub).OnIsSoundReady = OnIsSoundReady;
		FUNCTABLE(AssetHub).OnIsMusicReady = OnIsMusicReady;
		FUNCTABLE(AssetHub).OnIsFontReady = OnIsFontReady;
		FUNCTABLE(AssetHub).OnIsAnimReady = OnIsAnimReady;
		FUNCTABLE(AssetHub).GetTexture = OnGetTexture;
		FUNCTABLE(AssetHub).GetSound = OnGetSound;
		FUNCTABLE(AssetHub).GetMusic = OnGetMusic;
		FUNCTABLE(AssetHub).GetFont = OnGetFont;
		FUNCTABLE(AssetHub).GetAnim = OnGetAnim;
		FUNCTABLE(AssetHub).GetTextureSize = OnGetTextureSize;
		FUNCTABLE(AssetHub).GetFontSize = OnGetFontSize;
		FUNCTABLE(AssetHub).OnGetAssetLoadProgress = OnGetAssetLoadProgress;
		FUNCTABLE(AssetHub).OnGetFontAtlasCoord = OnGetFontAtlasCoord;
		FUNCTABLE(AssetHub).OnGetFontAtlasSrcRect = OnGetFontAtlasSrcRect;
		FUNCTABLE(AssetHub).OnGetFontAtlasAdvance = GetFontAtlasAdvance;
#pragma endregion

#pragma region Task
		FUNCTABLE(TaskScheduler).OnPostJob = OnPostJob;
#pragma endregion

#pragma region Input
		FUNCTABLE(InputMonitor).OnGetKeyDown = OnGetKeyDown;
		FUNCTABLE(InputMonitor).OnGetKeyPressed = OnGetKeyPressed;
		FUNCTABLE(InputMonitor).OnGetKeyReleased = OnGetKeyReleased;
		FUNCTABLE(InputMonitor).OnResetAllState = OnResetAllState;
		FUNCTABLE(InputMonitor).OnResetMouseState = OnResetMouseState;
		FUNCTABLE(InputMonitor).OnGetMousePosition = OnGetMousePosition;
		FUNCTABLE(InputMonitor).OnGetTextInputBuffer = OnGetTextInputBuffer;
		FUNCTABLE(InputMonitor).OnStartTextInput = OnStartTextInput;
		FUNCTABLE(InputMonitor).OnStopTextInput = OnStopTextInput;
		FUNCTABLE(InputMonitor).OnGetCompositionText = OnGetCompositionText;
		FUNCTABLE(InputMonitor).OnGetCompositionCursor = OnGetCompositionCursor;
		FUNCTABLE(InputMonitor).OnSetTextInputRect = OnSetTextInputRect;
#pragma endregion

#pragma region Config
		FUNCTABLE(Configurator).OnGetRenderAPI = OnGetRenderAPI;
		FUNCTABLE(Configurator).OnGetEnableVSync = OnGetEnableVSync;
		FUNCTABLE(Configurator).GetTileMap = OnGetTileMap;
		FUNCTABLE(Configurator).GetCharLayouts = OnGetCharLayouts;
#pragma endregion

#pragma region Window
		FUNCTABLE(Window).OnGetWindow = OnGetWindow;
		FUNCTABLE(Window).OnPollEvents = OnPollEvents;
		FUNCTABLE(Window).OnCloseWindow = OnCloseWindow;
		FUNCTABLE(Window).OnIsClose = OnIsClose;
		FUNCTABLE(Window).OnGetNativeWindow = OnGetNativeWindow;
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
		FUNCTABLE(GameWorld).OnSendJoinWorldRequest = OnSendJoinWorldRequest;
		FUNCTABLE(GameWorld).OnGetLoaclPlayerNetID = OnGetLocalPlayerNetId;
		FUNCTABLE(GameWorld).OnGetLocalPlayer = OnGetLocalPlayer;
#pragma endregion

#pragma region Net
		FUNCTABLE(NetworkClient).OnConnect = OnConnect;
		FUNCTABLE(NetworkClient).OnDisconnect = OnDisconnect;
		FUNCTABLE(NetworkClient).OnGetMessageQueue = OnGetMessageQueue;    // 现在带 PacketType 参数
		FUNCTABLE(NetworkClient).OnSendReliable = OnSendReliable;      // 新字段
		FUNCTABLE(NetworkClient).OnSendUnreliable = OnSendUnreliable;    // 新字段
		FUNCTABLE(NetworkClient).OnIsConnected = OnIsConnected;
		FUNCTABLE(NetworkClient).OnGetLocalConnId = OnGetLocalConnId;    // 新增 ID 访问
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
		if (!FUNCTABLE(Configurator).Check()) { throw std::runtime_error("Online [FuncTable] Miss [Configurator]!"); }
		if (!FUNCTABLE(Window).Check()) { throw std::runtime_error("Online [FuncTable] Miss [Window]!"); }
		if (!FUNCTABLE(AssetHub).Check()) { throw std::runtime_error("Online [FuncTable] Miss [AssetHub]!"); }
		if (!FUNCTABLE(EventDispatcher).Check()) { throw std::runtime_error("Online [FuncTable] Miss [EventDispatcher]!"); }
		if (!FUNCTABLE(InputMonitor).Check()) { throw std::runtime_error("Online [FuncTable] Miss [InputMonitor]!"); }
		if (!FUNCTABLE(Logger).Check()) { throw std::runtime_error("Online [FuncTable] Miss [Logger]!"); }
		if (!FUNCTABLE(ThreadTracker).Check()) { throw std::runtime_error("Online [FuncTable] Miss [ThreadTracker]!"); }
		if (!FUNCTABLE(Chronometer).Check()) { throw std::runtime_error("Online [FuncTable] Miss [Chronometer]!"); }
		if (!FUNCTABLE(TaskScheduler).Check()) { throw std::runtime_error("Online [FuncTable] Miss [TaskScheduler]!"); }
		if (!FUNCTABLE(LifeCycleTable).Check()) { throw std::runtime_error("Online [FuncTable] Miss [LifeCycleTable]!"); }
		if (!FUNCTABLE(GameWorld).Check()) { throw std::runtime_error("Online [FuncTable] Miss [GameWorld]!"); }
		if (!FUNCTABLE(NetworkClient).Check()) { throw std::runtime_error("Online [FuncTable] Miss [NetworkClient]!"); }
	} while (false);

	//Create Modules
	do
	{
		if (!MODULE(Configurator).Create()) { throw std::runtime_error("Online [Module] [Configurator] Create Fail!"); }
		if (!MODULE(Chronometer).Create()) { throw std::runtime_error("Online [Module] [Chronometer] reate Fail!"); }
		if (!MODULE(Logger).Create()) { throw std::runtime_error("Online [Module] [Logger] Create Fail!"); }
		if (!MODULE(Window).Create(MODULE(Configurator)->GetWindowPlatform())) { throw std::runtime_error("Online [Module] [Window] Create Fail!"); }
		if (!MODULE(ThreadTracker).Create()) { throw std::runtime_error("Online [Module] [ThreadTracker] Create Fail!"); }
		if (!MODULE(TaskScheduler).Create()) { throw std::runtime_error("Online [Module] [TaskScheduler] Create Fail!"); }
		if (!MODULE(EventDispatcher).Create()) { throw std::runtime_error("Online [Module] [EventDispatcher] Create Fail!"); }
		if (!MODULE(AssetHub).Create()) { throw std::runtime_error("Online [Module] [AssetHub] Create Fail!"); }
		if (!MODULE(InputMonitor).Create()) { throw std::runtime_error("Online [Module] [InputMonitor] Create Fail!"); }
		if (!MODULE(RenderPipeline).Create()) { throw std::runtime_error("Online [Module] [RenderPipeline] Create Fail!"); }
		if (!MODULE(Renderer).Create(MODULE(Configurator)->GetRenderAPI())) { throw std::runtime_error("Online [Module] [Renderer] Create Fail!"); }
		if (!MODULE(PhysicsSimulator).Create()) { throw std::runtime_error("Online [Module] [PhysicsSimulator] Create Fail!"); }
		if (!MODULE(LifeCycleTable).Create()) { throw std::runtime_error("Online [Module] [LifeCycleTable] Create Fail!"); }
		if (!MODULE(GameWorld).Create()) { throw std::runtime_error("Online [Module] [GameWorld] Create Fail!"); }
		if (!MODULE(NetworkClient).Create()) { throw std::runtime_error("Online [Module] [NetworkClient] Create Fail!"); }
		if (!MODULE(AudioPlayer).Create()) { throw std::runtime_error("Online [Module] [AudioPlayer] Create Fail!"); }
	} while (false);

	//Check Modules
	do
	{
		if (!MODULE(Configurator)) { throw std::runtime_error("Online [Module] Miss [Configurator]!"); }
		if (!MODULE(Chronometer)) { throw std::runtime_error("Online [Module] Miss [Chronometer]!"); }
		if (!MODULE(Logger)) { throw std::runtime_error("Online [Module] Miss [Logger]!"); }
		if (!MODULE(Window)) { throw std::runtime_error("Online [Module] Miss [Window]!"); }
		if (!MODULE(ThreadTracker)) { throw std::runtime_error("Online [Module] Miss [ThreadTracker]!"); }
		if (!MODULE(EventDispatcher)) { throw std::runtime_error("Online [Module] Miss [EventDispatcher]!"); }
		if (!MODULE(AssetHub)) { throw std::runtime_error("Online [Module] Miss [AssetHub]!"); }
		if (!MODULE(InputMonitor)) { throw std::runtime_error("Online [Module] Miss [InputMonitor]!"); }
		if (!MODULE(RenderPipeline)) { throw std::runtime_error("Online [Module] Miss [RenderPipeline]!"); }
		if (!MODULE(Renderer)) { throw std::runtime_error("Online [Module] Miss [Renderer]!"); }
		if (!MODULE(PhysicsSimulator)) { throw std::runtime_error("Online [Module] Miss [PhysicsSimulator]!"); }
		if (!MODULE(TaskScheduler)) { throw std::runtime_error("Online [Module] Miss [TaskScheduler]!"); }
		if (!MODULE(LifeCycleTable)) { throw std::runtime_error("Online [Module] Miss [LifeCycleTable]!"); }
		if (!MODULE(GameWorld)) { throw std::runtime_error("Online [Module] Miss [GameWorld]!"); }
		if (!MODULE(NetworkClient)) { throw std::runtime_error("Online [Module] Miss [NetworkClient]!"); }
		if (!MODULE(AudioPlayer)) { throw std::runtime_error("Online [Module] Miss [AudioPlayer]!"); }
	} while (false);

	//Initialize Modules
	do
	{
		if (!MODULE(Configurator).Initialize()) { throw std::runtime_error("Online [Configurator] Module Initialize Fail!"); }
		if (!MODULE(Chronometer).Initialize(MODULE(Configurator)->GetEnableVSync())) { throw std::runtime_error("Online [Chronometer] Module Initialize Fail!"); }
		if (!MODULE(ThreadTracker).Initialize()) { throw std::runtime_error("Online [ThreadTracker] Module Initialize Fail!"); }
		if (!MODULE(Logger).Initialize()) { throw std::runtime_error("Online [Logger] Module Initialize Fail!"); }
		if (!MODULE(TaskScheduler).Initialize()) { throw std::runtime_error("Online [TaskScheduler] Module Initialize Fail!"); }
		if (!MODULE(EventDispatcher).Initialize()) { throw std::runtime_error("Online [EventDispatcher] Module Initialize Fail!"); }
		if (!MODULE(Window).Initialize(MODULE(Configurator)->GetWindowWidth(), MODULE(Configurator)->GetWindowHeight(), MODULE(Configurator)->GetWindowName().c_str())) { throw std::runtime_error("Online [Window] Module Initialize Fail!"); }
		if (!MODULE(AssetHub).Initialize(MODULE(Configurator)->GetAnimations(), MODULE(Configurator)->GetTileMaps())) { throw std::runtime_error("Online [AssetHub] Module Initialize Fail!"); }
		if (!MODULE(LifeCycleTable).Initialize()) { throw std::runtime_error("Online [LifeCycleTable] Module Initialize Fail!"); }
		if (!MODULE(GameWorld).Initialize()) { throw std::runtime_error("Online [GameWorld] Module Initialize Fail!"); }
		if (!MODULE(InputMonitor).Initialize()) { throw std::runtime_error("Online [InputMonitor] Module Initialize Fail!"); }
		if (!MODULE(RenderPipeline).Initialize()) { throw std::runtime_error("[RenderPipeline] Module Initialize Fail!"); }
		if (!MODULE(Renderer).Initialize(MODULE(Window)->GetNativeWindow())) { throw std::runtime_error("[Renderer] Module Initialize Fail!"); }
		if (!MODULE(PhysicsSimulator).Initialize()) { throw std::runtime_error("Online [PhysicsSimulator] Module Initialize Fail!"); }
		if (!MODULE(NetworkClient).Initialize()) { throw std::runtime_error("Online [NetworkClient] Module Initialize Fail!"); }
		if (!MODULE(AudioPlayer).Initialize()) { throw std::runtime_error("Online [AudioPlayer] Module Initialize Fail!"); }

	} while (false);
	// OnConnect("127.0.0.1",7777);

	return true;
}
bool Online::Runtime::Client::IsRunning() const
{
	return !MODULE(Window)->IsClose();
}
void Online::Runtime::Client::BeginFrame()
{
	MODULE(Chronometer)->Tick();
	MODULE(InputMonitor)->PrepareNewFrame();
	MODULE(Window)->PollEvents();
	MODULE(InputMonitor)->UpdateSnapshots();
	MODULE(AudioPlayer).BeginFrame();
}
void Online::Runtime::Client::FixedUpdate()
{
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
				col.GetRadius() * ((transform.GetWorldScale().x + transform.GetWorldScale().y) * 0.5f) , 
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
void Online::Runtime::Client::Update()
{
	MODULE(GameWorld).Update();
}
void Online::Runtime::Client::LateUpdate()
{
	MODULE(GameWorld).LateUpdate();
	if (auto* scene = MODULE(GameWorld)->GetActiveScene())
	{
		for (auto [entity, transform, audiosource] : scene->GetView<Online::Game::Transform, Online::Game::AudioSource>().each())
		{
			Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
			if (!obj || !obj->IsActive())
			{
				MODULE(AudioPlayer)->SubmitSound(
					static_cast<uint32_t>(entt::to_integral(entity)),
					glm::vec2{},
					Asset::SoundID::Invalid,
					0.0f, false, false, 0.0f,
					Audio::AudioQueue::WorldSFX
				);
				continue;
			}
			MODULE(AudioPlayer)->SubmitSound(
				static_cast<uint32_t>(entt::to_integral(entity)),
				transform.GetWorldPosition(),
				audiosource.GetSoundID(),
				audiosource.GetVolume(),
				audiosource.IsLoop(),
				audiosource.GetPlaying(),
				audiosource.GetSpatialBlend(),
				audiosource.GetPriority()
			);
		}
		
		for (auto [entity, transform, audiolisten] : scene->GetView<Online::Game::Transform, Online::Game::AudioListener>().each())
		{
			Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
			if (!obj || !obj->IsActive())
			{
				continue;
			}
			Online::Core::ListenSnapshot snapshot(transform.GetWorldPosition());
			snapshot.Range = audiolisten.GetRange();
			snapshot.MasterVolume = audiolisten.GetMasterVolume();
			MODULE(AudioPlayer)->SubmitListener(snapshot);

			MODULE(AudioPlayer)->SubmitBackgroundMusic(
				audiolisten.GetVackGroundID(),
				audiolisten.GetMusicVolum(),
				audiolisten.GetPaused()
			);

			break;
		}
	}
}
void Online::Runtime::Client::Render()
{
	MODULE(RenderPipeline)->NewPipeline();

	if (auto* scene = MODULE(GameWorld)->GetActiveScene())
	{
		for (auto [entity, transform, sprite] : scene->GetView<Online::Game::Transform, Online::Game::Sprite>().each())
		{
			Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
			if (!obj || !obj->IsActive())
			{
				continue;
			}

			if (!sprite.IsVisible())
			{
				continue;
			}

			MODULE(RenderPipeline)->AddRenderItem(
#pragma region AddRenderItem
				obj->GetLayerMask(),
				sprite.GetTexture(),
				sprite.GetRenderQueue(),
				sprite.GetSrcRect(),
				sprite.GetDstRect(transform.GetWorldPosition(), transform.GetWorldScale()),
				sprite.GetDrawOrder(),
				sprite.GetDepth(),
				transform.GetWorldRotation(),
				sprite.GetPivot(),
				sprite.GetFlip(),
				sprite.GetColor()
#pragma endregion
			);
		} 

		for (auto [entity, transform, textComp] : scene->GetView<Online::Game::Transform, Online::Game::Text>().each())
		{
			Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
			if (!obj || !obj->IsActive() || !textComp.IsVisible())
				continue;

			MODULE(RenderPipeline)->AddRenderText(
				obj->GetLayerMask(),
				textComp.GetFont(),
				textComp.GetText(),
				textComp.GetRenderQueue(),
				textComp.GetDrawOrder(),
				textComp.GetDepth(),
				transform.GetWorldPosition() + textComp.GetOffset(),
				transform.GetWorldScaleAverage(),
				transform.GetWorldRotation() + textComp.GetRotation(),
				textComp.GetPivot(),
				textComp.GetColor(),
				textComp.GetAnchor(),
				textComp.GetLetterSpacing(),
				textComp.GetWidthLimit()/*,
				textComp.GetPixelOffset(),
				textComp.GetMaxPixelWidth()*/
			);
		}

		for (auto [entity, transform, camera] : scene->GetView<Online::Game::Transform, Online::Game::Camera>().each())
		{
			Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
			if (!obj || !obj->IsActive())
			{
				continue;
			}

			if (camera.IsRenderTargetReady())
			{
				MODULE(RenderPipeline)->AddRenderPass(
					camera.GetCameraState().BuildSnapshot(transform.GetWorldPosition()),
					camera.GetRenderTarget(), camera.GetPostProcessSetting(), camera.GetCullingMask());
			}
		}

#if _DEBUG 
		auto debugSegments = MODULE(PhysicsSimulator)->GetDebugDrawData();
		for (auto& seg : debugSegments)
		{
			MODULE(Renderer)->SubmitLine(seg.p1, seg.p2, seg.color);
		}
#endif

		for (auto [entity, transform, tilemap] : scene->GetView<Online::Game::Transform, Online::Game::TileMap>().each())
		{
			Online::Game::GameObject* obj = Online::Game::GetGameObject(entity);
			if (!obj || !obj->IsActive())
			{
				continue;
			}

			MODULE(RenderPipeline)->AddRenderItem(
				tilemap.GetLayerMask(),
				tilemap.GetTexture(),
				tilemap.GetRenderQueue(),
				tilemap.GetSize(),
				tilemap.GetMapSize(transform.GetWorldPosition(), transform.GetWorldScale()),
				transform.GetWorldRotation()
			);
		}


	}

	MODULE(RenderPipeline)->Execute(MODULE(Renderer).Get());
}
void Online::Runtime::Client::EndFrame()
{
	MODULE(GameWorld).EndFrame();
	MODULE(AudioPlayer).EndFrame();
}
void Online::Runtime::Client::FrameSync()
{
	MODULE(Chronometer)->FrameSync();
}
void Online::Runtime::Client::Release()
{
	//Release Modules
	do
	{
		MODULE(AudioPlayer).Release();
		MODULE(NetworkClient).Release();
		MODULE(Configurator).Release();
		MODULE(Chronometer).Release();
		MODULE(InputMonitor).Release();
		MODULE(GameWorld).Release();
		MODULE(PhysicsSimulator).Release();
		MODULE(RenderPipeline).Release();
		MODULE(Renderer).Release();
		MODULE(Window).Release();
		MODULE(AssetHub).Release();
		MODULE(EventDispatcher).Release();
		MODULE(TaskScheduler).Release();
		MODULE(Logger).Release();
		MODULE(ThreadTracker).Release();
		MODULE(LifeCycleTable).Release();
	} while (false);

	//Destroy Modules
	do
	{
		MODULE(NetworkClient).Destroy();
		MODULE(GameWorld).Destroy();
		MODULE(Chronometer).Destroy();
		MODULE(InputMonitor).Destroy();
		MODULE(PhysicsSimulator).Destroy();
		MODULE(RenderPipeline).Destroy();
		MODULE(Renderer).Destroy();
		MODULE(Configurator).Destroy();
		MODULE(Window).Destroy();
		MODULE(AssetHub).Destroy();
		MODULE(EventDispatcher).Destroy();
		MODULE(TaskScheduler).Destroy();
		MODULE(Logger).Destroy();
		MODULE(ThreadTracker).Destroy();
		MODULE(LifeCycleTable).Destroy();
	} while (false);

	//UnRegister FuncTables
	do
	{
		FUNCTABLE(NetworkClient).UnRegister();
		FUNCTABLE(GameWorld).UnRegister();
		FUNCTABLE(Configurator).UnRegister();
		FUNCTABLE(AssetHub).UnRegister();
		FUNCTABLE(EventDispatcher).UnRegister();;
		FUNCTABLE(InputMonitor).UnRegister();
		FUNCTABLE(TaskScheduler).UnRegister();
		FUNCTABLE(Logger).UnRegister();
		FUNCTABLE(Chronometer).UnRegister();
		FUNCTABLE(Window).UnRegister();
		FUNCTABLE(ThreadTracker).UnRegister();
		FUNCTABLE(LifeCycleTable).UnRegister();
	} while (false);

	//UnRegister Context
	Online::Runtime::Context::Instance().UnRegister();
	Online::Runtime::ClientContext::Instance().UnRegister();
}
#undef MODULE
#undef FUNCTABLE
template class Online::Core::Singleton<Online::Runtime::Client>;