#include<Core/Utils/File.h>
#include<Game/GameWorld.h>
#include<Game/Scene/Scene.h>
#include<Game/Entity/GameObject.h>  
#include<Time/Common/FuncTable.h>
#include<Input/Common/FuncTable.h>
#include<Task/Common/FuncTable.h>
#include<Net/Common/WorldSnapshot.h>
#include<Net/Common/ExitWorldPacket.h>
#include<Net/Common/EntityDestroyPacket.h>
#include<Event/Common/FuncTable.h>

#include<chrono>
#include<thread> 

#ifdef PIXEL_CLIENT
#include<Script/LifeCycleFunc/Follow.h>
#include<Script/LifeCycleFunc/Button.h>
#include<Script/LifeCycleFunc/TextInput.h>
#include<Net/Client/Common/FuncTable.h>
#include<Net/Common/RespEntityDataPacket.h>
#endif

#ifdef PIXEL_SERVER
#include<Net/Server/Common/FuncTable.h>
#include<Net/Common/ReqEntityDataPacket.h>
#include<Net/Common/RespEntityDataPacket.h>
#include<Net/Common/EntityFullData.h>
#endif // PIXEL_SERVER


bool Online::Game::GameWorld::Initialize()
{
#ifdef PIXEL_CLIENT
	BuildSettings["BeginScene"] = Core::GetExeDir() + "scenes\\begin.json";
	BuildSettings["LoadingScene"] = Core::GetExeDir() + "scenes\\loading.json";
	BuildSettings["StartScene"] = Core::GetExeDir() + "scenes\\start.json";

	loadingScene = ONLINE_NEW(Scene);
	if (!loadingScene->DeserializeFromFile(BuildSettings["LoadingScene"], Serialize::API::Json))
	{
		ONLINE_DELETE(loadingScene);
		loadingScene = nullptr;
		throw std::runtime_error("Failed to load loading scene");
	}

	activeScene = ONLINE_NEW(Scene);

	if (activeScene)
	{
		bool ok = activeScene->DeserializeFromFile(BuildSettings["BeginScene"], Online::Serialize::API::Json);
		if (!ok)
			Log::Error("Failed to load begin scene");
	}

	/*GameObject* mainCamera = activeScene->CreateGameObject("Main Camera", "MainCamera");
	Camera& camera = mainCamera->AddComponent<Camera>();
	camera.SetRenderTarget(Online::Asset::TextureID::Tex_WindowBuffer);
	camera.SetRenderSize({ 1280, 780 });
	camera.SetZone(0.5f);
	mainCamera->GetTransform()->SetLocalPosition({ 0.0f, 0.0f });

	GameObject* UICamera = activeScene->CreateGameObject("UI Camera", "UICamera");
	Camera& UIcamera = UICamera->AddComponent<Camera>();
	UIcamera.SetRenderTarget(Online::Asset::TextureID::Tex_BackBuffer_1);
	UIcamera.SetRenderSize({ 1280, 780 });
	UIcamera.SetCullingMask(Render::RenderLayer::UI);
	UIcamera.SetClearColor(Core::Color::Clear);
	UIcamera.SetIsWorld(false);
	UICamera->GetTransform()->SetLocalPosition({ 0.0f, 0.0f });


	GameObject* phys = activeScene->CreateGameObject("dog");
	Transform* wuli = phys->GetComponent<Transform>();
	wuli->SetLocalScale({ 4,4 });
	wuli->SetLocalPosition({ 700,800 });

	Sprite& mainSprite = phys->AddComponent<Sprite>();
	Animator& mainAnim = phys->AddComponent<Animator>();
	mainSprite.SetRenderOffset({ 0,-58 });
	mainSprite.SetRenderQueue(Render::RenderQueue::World);
	mainAnim.SetSprite(&mainSprite);

	GameObject* overlayGO = activeScene->CreateGameObject("Overlay");
	overlayGO->SetParent(phys);
	Sprite& overlaySprite = overlayGO->AddComponent<Sprite>();
	Animator& overlayAnim = overlayGO->AddComponent<Animator>();
	overlaySprite.SetRenderOffset({ 0,-55 });
	overlayAnim.SetSprite(&overlaySprite);
	overlaySprite.OnDisable();

	AnimatorController& controller = phys->AddComponent<AnimatorController>();
	controller.SetStates({
		{ "Idle", Asset::AnimationClipID::Anim_SilverHat_Idle },
		{ "Run",  Asset::AnimationClipID::Anim_SilverHat_Run }
		});
	controller.SetMainAnimator(&mainAnim);
	controller.SetOverlayAnimator(&overlayAnim);

	controller.SetTransitions({
	{
		"Idle",
		"Run",
		0.0f,
		{ { "Speed", AnimatorConditionMode::Greater, 0.1f } }
	},
	{
		"Run",
		"Idle",
		0.0f,
		{ { "Speed", AnimatorConditionMode::Less, 0.1f } }
	}
		});

	controller.SetDefaultStateName("Idle");
	controller.Play("Idle");

	Collider& col = phys->AddComponent<Collider>();
	col.SetShape(Physics::ColliderShape::Capsule);
	col.SetHalfSize({ 12,6 });
	col.SetRadius(12);
	col.SetCategoryBits(Physics::PhysicsLayer::Player);
	Rigidbody& rb = phys->AddComponent<Rigidbody>();
	rb.SetGravityScale(1.5);
	rb.SetFixedRotation(true);
	phys->AddScriptFunction(Script::ScriptFunctionID::MoveLeftRight);

	Follow& ff = mainCamera->AddComponent<Follow>();
	ff.SetTarget(wuli);
	ff.SetFollowMode(FollowMode::Linear);
	ff.SetLinearSpeed(10.0f);
	ff.SetOffest({ -400, -1000 });

	GameObject* tri = activeScene->CreateGameObject("Tri");
	Transform* tritrans = tri->GetComponent<Transform>();
	tritrans->SetLocalPosition({ 1200,1200 });
	tritrans->SetLocalScale({ 2,2 });
	Collider& tricoll = tri->AddComponent<Collider>();
	tricoll.SetTrigger(true);

	GameObject* tri1 = activeScene->CreateGameObject("Tri1");
	Transform* tritrans1 = tri1->GetComponent<Transform>();
	tritrans1->SetLocalPosition({ 300,1200 });
	tritrans1->SetLocalScale({ 2,2 });
	Collider& tricoll1 = tri1->AddComponent<Collider>();
	tricoll1.SetTrigger(true);
	Rigidbody& tirrigi1 = tri1->AddComponent<Rigidbody>();
	tirrigi1.SetBodyType(Physics::BodyType::Kinematic);

	GameObject* tile = activeScene->CreateGameObject("Tile");
	Transform* tiletrans = tile->GetComponent<Transform>();
	tiletrans->SetLocalScale({ 4,4 });
	TileMap& tilemap = tile->AddComponent<TileMap>();
	tilemap.SetTileMapID(Config::TileMapID::Map_01);
	tilemap.SetRenderQueue(Render::RenderQueue::Background);


	GameObject* butone = activeScene->CreateGameObject("butone");
	butone->SetLayer(Render::RenderLayer::UI);
	Transform* butonetrans = butone->GetComponent<Transform>();
	butonetrans->SetLocalScale({ 3,3 });
	butonetrans->SetLocalPosition({ 100,100 });
	Sprite& butonesprite = butone->AddComponent<Sprite>();
	butonesprite.SetTexture(Asset::TextureID::Tex_Flag);
	butonesprite.SetGrid(6,1);
	butone->AddScriptFunction(Script::ScriptFunctionID::Button);*/

	/*auto* buttonData = butone->GetScriptData<Script::Button::ButtonData>(Script::ScriptFunctionID::Button);
	if (buttonData)
	{
		buttonData->SetOnClick([phys](Game::GameObject* sender) {
			if (phys)
			{
				auto* trans = phys->GetTransform();
				if (trans)
					trans->SetWorldPosition({ 700.0f, 800.0f });
			}
			});
	}

	GameObject* inputObj = activeScene->CreateGameObject("NameInput");
	inputObj->SetLayer(Render::RenderLayer::UI);
	Transform* inputTrans = inputObj->GetTransform();
	inputTrans->SetLocalPosition({ 300, 200 });
	inputTrans->SetLocalScale({ 1, 1 });
	Sprite& inputBk = inputObj->AddComponent<Sprite>();
	inputBk.SetTexture(Asset::TextureID::Tex_Flag);
	inputBk.SetAnchor(Core::Anchor::CenterLeft);

	GameObject* textCursor = activeScene->CreateGameObject("Cursor");
	textCursor->SetLayer(Render::RenderLayer::UI);
	textCursor->AddComponent<Sprite>().SetTexture(Asset::TextureID::Tex_TextCursor);

	Text& inputText = inputObj->AddComponent<Text>();
	inputText.SetFont(Asset::FontID::Ipix);
	inputText.SetRenderQueue(Render::RenderQueue::UI);
	inputText.SetAnchor(Core::Anchor::CenterLeft);
	inputText.SetWidthLimit(144);

	inputObj->AddScriptFunction(Script::ScriptFunctionID::TextInput);
	auto* inputData = inputObj->GetScriptData<Script::TextInput::TextInputData>(Script::ScriptFunctionID::TextInput);
	if (inputData)
	{
		inputData->maxChars = 16;
		inputData->placeholder = "Entername...";
		inputData->SetOnTextChanged([](Game::GameObject* go, const std::string& text) {
			Online::Log::Debug("TextInput changed: " + text);
			});
		inputData->SetOnEnterPressed([](Game::GameObject* go, const std::string& text) {
			Online::Log::Debug("TextInput entered: " + text);
			});
	}

	GameObject* bkFar = activeScene->CreateGameObject("Background Far");
	bkFar->SetLayer(Render::RenderLayer::Default);
	Transform* bkFarTrans = bkFar->GetComponent<Transform>();
	bkFarTrans->SetLocalPosition({ 0,0 });
	bkFarTrans->SetLocalScale({ 8,8 });
	Sprite& bkFarSprite = bkFar->AddComponent<Sprite>();
	bkFarSprite.SetAnchor(Core::Anchor::TopLeft);
	bkFarSprite.SetTexture(Asset::TextureID::Tex_BackGround_Far);
	bkFarSprite.SetRenderQueue(Render::RenderQueue::Background);
	bkFar->AddScriptFunction(Script::ScriptFunctionID::FollowOverTime);
	bkFar->GetScriptData<Script::Follow::FollowData>(Script::ScriptFunctionID::FollowOverTime)->SetTarget(mainCamera->GetComponent<Transform>());

	GameObject* UICanvas = activeScene->CreateGameObject("Canvas");
	Sprite& uisprite = UICanvas->AddComponent<Sprite>();
	uisprite.SetTexture(Asset::TextureID::Tex_BackBuffer_1);
	uisprite.SetAnchor(Core::Anchor::TopLeft);
	UICanvas->AddScriptFunction(Script::ScriptFunctionID::FollowOverTime);
	UICanvas->GetScriptData<Script::Follow::FollowData>(Script::ScriptFunctionID::FollowOverTime)->SetTarget(mainCamera->GetComponent<Transform>());*/
#endif


#ifdef PIXEL_SERVER
 
    StepCompletedToken = Online::Event::Subscribe(
	    Online::Event::EventType::PhysStepCompleted,
	    [](void* listener, const Online::Event::Event&) {
		    auto* gw = static_cast<GameWorld*>(listener);
	    	gw->serverFrame++;
		    if (gw->activeScene)
			    gw->activeScene->BroadcastEntityStates();
	    },
	    this
    );

	serverWorld = ONLINE_NEW(Scene);
	activeScene = serverWorld;

	GameObject* tile = serverWorld->CreateGameObject("Tile");
	Transform* tiletrans = tile->GetComponent<Transform>();
	tiletrans->SetLocalScale({ 4,4 });
	TileMap& tilemap = tile->AddComponent<TileMap>();
	tilemap.SetTileMapID(Config::TileMapID::Map_01);
	tilemap.SetRenderQueue(Render::RenderQueue::Background);
	SyncTransform* syncTrans = tile->GetComponent<SyncTransform>();
	syncTrans->OnDisable();
#endif // PIXEL_SERVER

	return true;
}

void Online::Game::GameWorld::Release()
{
	UnloadCurrentScene();
	if (StepCompletedToken.type != Online::Event::EventType::Invalid)
		Online::Event::UnSubscribe(StepCompletedToken);
	if (loadingScene)
	{
		ONLINE_DELETE(loadingScene);
		loadingScene = nullptr;
	}
	if(pendingScene)
	{
		ONLINE_DELETE(pendingScene);
		pendingScene = nullptr;
	}
#ifdef PIXEL_CLIENT


	Net::ExitWorldNotice notice;
	notice.ClientID = Net::Client::GetLocalConnId();

	std::vector<std::byte> payload = notice.SerializePayload();
	Net::Client::SendReliable(payload, notice.TYPE, Online::Net::ChannelType::ReliableUnordered);
#endif // ONLINE_CLIENT

#ifdef PIXEL_SERVER
	if (serverWorld)
	{
		ONLINE_DELETE(serverWorld);
		serverWorld = nullptr;
	}
#endif
	localPlayerNetId = -1;
	isAsyncLoading = false;
	pendingApplyScene = false;
}

void Online::Game::GameWorld::FixedUpdate()
{
	if (activeScene)
	{
		activeScene->ProcessTransformDirtySystem();
		activeScene->ProcessColliderSystem();
		activeScene->ProcessColliderListSystem();
	}

#ifdef PIXEL_SERVER

	auto& msgQueue = Net::Server::GetMessageQueue(Net::PacketType::JoinWorldRequest);
	Net::NetMessage msg;

	while (msgQueue.Pop(msg))
	{
		Net::JoinWorldRequest req;
		if (req.DeserializeFromPayload(msg.body))
		{
			HandleJoinWorldRequest(msg.connectionId, req);
		}
	}

	auto& reqDataQueue = Net::Server::GetMessageQueue(Net::PacketType::ReqEntityData);
	Net::NetMessage reqMsg;
	while (reqDataQueue.Pop(reqMsg))
	{
		Net::ReqEntityDataPacket req;
		if (req.DeserializeFromPayload(reqMsg.body))
		{
			HandleEntityDataRequest(reqMsg.connectionId, req);
		}
	}

	auto& ExitNoticeQueue = Net::Server::GetMessageQueue(Net::PacketType::ExitWorldNotice);
	Net::NetMessage ExitMsg;
	while (ExitNoticeQueue.Pop(ExitMsg))
	{
		Net::ExitWorldNotice notic;
		if (notic.DeserializeFromPayload(ExitMsg.body))
		{
			HandleExitWorldNotice(notic.ClientID);
		}
	}
#endif

}

void Online::Game::GameWorld::Update()
{
	if (activeScene)
		activeScene->Update(Time::delta());
}

void Online::Game::GameWorld::LateUpdate()
{

	if (pendingApplyScene)
	{
		ApplyLoadedScene(pendingScene);
		pendingApplyScene = false;
		pendingScene = nullptr;
		isAsyncLoading = false;
	}
#ifdef PIXEL_CLIENT
	auto& snapshotQueue = Net::Client::GetMessageQueue(Net::PacketType::WorldSnapshot);
	Net::NetMessage snapshotMsg;

	while (snapshotQueue.Pop(snapshotMsg))
	{
		const auto& snapshotBin = snapshotMsg.body;
		Online::Log::Info("收到WorldSnapshot数据，总长度: " + std::to_string(snapshotBin.size()) + " bytes");

		Net::WorldSnapshot snapshot;
		if (!snapshot.DeserializeFromPayload(snapshotBin))
		{
			Online::Log::Error("解析WorldSnapshot业务包失败");
			continue;
		}
		localPlayerNetId = snapshot.localPlayerNetId;
		Online::Log::Info("本地玩家NetID: " + std::to_string(localPlayerNetId));

		LoadScene(snapshot.sceneData);
	}

	if (activeScene)
	{
		auto& msgQueue = Net::Client::GetMessageQueue(Net::PacketType::EntityState);
		Net::NetMessage msg;

		while (msgQueue.Pop(msg))
		{
			Net::EntityStatePacket pkt;
			if (pkt.DeserializeFromPayload(msg.body))
			{
				activeScene->ProcessEntityStatePacket(pkt);
			}
		}

		auto& respQueue = Net::Client::GetMessageQueue(Net::PacketType::RespEntityData);
		Net::NetMessage respMsg;
		while (respQueue.Pop(respMsg))
		{
			Net::RespEntityDataPacket resp;
			if (resp.DeserializeFromPayload(respMsg.body))
			{
				activeScene->CreateEntityFromFullData(resp.entityData);
			}
		}

		auto& destroyQueue = Net::Client::GetMessageQueue(Net::PacketType::EntityDestroy);
		Net::NetMessage destroyMsg;
		while (destroyQueue.Pop(destroyMsg))
		{
			Net::EntityDestroyPacket pkt;
			if (pkt.DeserializeFromPayload(destroyMsg.body))
			{
				for (uint32_t netId : pkt.netIds)
				{
					auto view = activeScene->ecsRegistry.view<NetID>();
					for (auto entity : view)
					{
						if (view.get<NetID>(entity).GetNetId() == netId)
						{
							activeScene->DestroyEntity(entity);
							break;
						}
					}
				}
			}
		}
	}

#endif

	

	if (activeScene)
	{
		activeScene->ProcessFollowSystem(Online::Time::delta());
		activeScene->ProcessAnimationSystem(Online::Time::delta());
		activeScene->ProcessAnimatorControllerSystem(Online::Time::delta());
		activeScene->ProcessProgressBarSystem(Online::Time::delta());
	}

	if (activeScene)
		activeScene->LateUpdate(Time::delta());


#ifdef PIXEL_CLIENT	
	if (activeScene)
        activeScene->ProcessSyncTransform();
#endif
}

void Online::Game::GameWorld::EndFrame()
{

	if (activeScene) { activeScene->ProcessDelayDestroyQueue(); }
}

void Online::Game::GameWorld::UnloadCurrentScene()
{
	if (activeScene == nullptr) return;

	activeScene->ProcessDelayDestroyQueue();
	Input::ResetAllState();

	ONLINE_DELETE(activeScene);
	activeScene = nullptr;
	showLoadingScene = false;
	Online::Log::Info("GameWorld: Unloaded current scene successfully");
}

void Online::Game::GameWorld::AddAnimatorControll(Game::RoleID RoleID, GameObject* player)
{
	if (!player)
		return;
	switch (RoleID)
	{
	case Game::RoleID::SilverHat:
		AddAnimatorControllForSilverHat(player);
		break;
	case Game::RoleID::RedGeneral:
		break;
	default:
		AddAnimatorControllForSilverHat(player);
	}
}

void Online::Game::GameWorld::LoadScene(const std::string& sceneName)
{
	if (isAsyncLoading)
	{
		Log::Warning("Already switching scene");
		return;
	}

	if (BuildSettings.find(sceneName) == BuildSettings.end())
	{
		Online::Log::Error("GameWorld: LoadScene failed - not find " + sceneName);
		return;
	}

	isAsyncLoading = true;

	Task::PostJob([this, sceneName]() {

		std::string path = BuildSettings[sceneName];
		Scene* scene = ONLINE_NEW(Scene);
		bool ok = scene->DeserializeFromFile(path, Serialize::API::Json);

		if (!ok)
		{
			ONLINE_DELETE(scene);
			scene = nullptr;
		}

		if(pendingScene)
		{
			ONLINE_DELETE(pendingScene);
			pendingScene = nullptr;
		}
		isAsyncLoading = false;
		pendingScene = scene;

		}, "SceneAsyncLoad" + sceneName);

	return;
}

void Online::Game::GameWorld::LoadScene(const std::vector<std::byte>& sceneByte)
{
	if (isAsyncLoading)
	{
		Log::Warning("Already switching scene");
		return;
	}

	isAsyncLoading = true;

	Task::PostJob([this, sceneByte]() {

		Scene* scene = ONLINE_NEW(Scene);
		bool ok = scene->DeserializeFromBytes(sceneByte, Serialize::API::Json);

		if (!ok)
		{
			ONLINE_DELETE(scene);
			scene = nullptr;
		}

		if(pendingScene)
		{
			ONLINE_DELETE(pendingScene);
			pendingScene = nullptr;
		}

		isAsyncLoading = false;
		pendingScene = scene;

		}, "SceneAsyncLoadFormByte" );

}

void Online::Game::GameWorld::SwitchSceneAfterLoadingAsync(const std::string& newSceneName)
{
	if (isAsyncLoading)
	{
		Log::Warning("Already switching scene");
		return;
	}

	if (!BuildSettings.contains(newSceneName))
	{
		Log::Error("Scene not found: " + newSceneName);
		return;
	}

	isAsyncLoading = true;
	UnloadCurrentScene();

	if (loadingScene)
	{
		ResetLoadingSceneState();
		activeScene = loadingScene;
		showLoadingScene = true;
	}

	Task::PostJob([this, newSceneName]() {

		const auto taskStartTime = std::chrono::steady_clock::now();
		const auto minWaitDuration = std::chrono::seconds(1);

		std::string path = BuildSettings[newSceneName];
		Scene* scene = ONLINE_NEW(Scene);
		bool ok = scene->DeserializeFromFile(path, Serialize::API::Json);

		if (!ok)
		{
			ONLINE_DELETE(scene);
			scene = nullptr;
		}

		const auto elapsedTime = std::chrono::steady_clock::now() - taskStartTime;

		if (elapsedTime < minWaitDuration)
		{
			const auto remainingTime = minWaitDuration - elapsedTime;
			std::this_thread::sleep_for(remainingTime);
		}

		if(pendingScene)
		{
			ONLINE_DELETE(pendingScene);
			pendingScene = nullptr;
		}

		pendingScene = scene;
		pendingApplyScene = true;

		}, "SceneAsyncAfterLoading" + newSceneName);
}

void Online::Game::GameWorld::SwitchSceneAsync(const std::string& newSceneName)
{
	if (isAsyncLoading)
	{
		Log::Warning("Already switching scene");
		return;
	}

	if (!BuildSettings.contains(newSceneName))
	{
		Log::Error("Scene not found: " + newSceneName);
		return;
	}

	isAsyncLoading = true;

	Task::PostJob([this, newSceneName]() {

		std::string path = BuildSettings[newSceneName];

		if (newSceneName == "LoadingScene")
		{
			pendingScene = loadingScene;
			pendingApplyScene = true;

			return;
		}

		Scene* scene = ONLINE_NEW(Scene);
		bool ok = scene->DeserializeFromFile(path, Serialize::API::Json);

		if (!ok)
		{
			ONLINE_DELETE(scene);
			scene = nullptr;
		}

		if (pendingScene)
		{
			ONLINE_DELETE(pendingScene);
			pendingScene = nullptr;
		}

		pendingScene = scene;

		}, "SceneAsyncLoad" + newSceneName);

}

void Online::Game::GameWorld::DisplayPendingScene()
{
	if (!pendingScene)
	{
		Log::Error("PendingScene is empty");
		return;
	}

	pendingApplyScene = true;
}

void Online::Game::GameWorld::SendJoinWorldRequest(const std::string& playerName, uint32_t playerId)
{
#ifdef PIXEL_CLIENT
	Net::JoinWorldRequest req;
	req.playerName = playerName;
	req.playerId = playerId;

	std::vector<std::byte> payload = req.SerializePayload();

	Net::Client::SendReliable(
		payload,
		Net::PacketType::JoinWorldRequest,
		Net::ChannelType::ReliableOrdered
	);

	Online::Log::Info("Sent join world request: " + playerName);
#endif
}

void Online::Game::GameWorld::HandleJoinWorldRequest(int connId, const Net::JoinWorldRequest& req)
{
#ifdef PIXEL_SERVER
	GameObject* player = serverWorld->CreateGameObject(req.playerName, "Player");

	static int x = 700;

	player->GetTransform()->SetWorldPosition({ x, 800 });
	player->GetTransform()->SetLocalScale({ 4, 4 });
	x += 100;

	Rigidbody& rb = player->AddComponent<Rigidbody>();
	rb.SetGravityScale(1.5f);
	rb.SetFixedRotation(true);
	rb.SetBodyType(Physics::BodyType::Dynamic);

	Collider& col = player->AddComponent<Collider>();
	col.SetShape(Physics::ColliderShape::Capsule);
	col.SetHalfSize({ 12.0f, 6.0f });
	col.SetRadius(12.0f);
	col.SetCategoryBits(Physics::PhysicsLayer::Player);	
	col.SetDensity(1.0f);
	col.SetFriction(0.3f);
	col.SetRestitution(0.0f);

	NetID* netId = player->GetComponent<NetID>();
	assert(netId);
	netId->SetOwnerConnId(connId);
	netId->SetNeedSync(true);

	AddAnimatorControllForSilverHat(player);

	player->AddScriptFunction(Script::ScriptFunctionID::MoveLeftRight);

	SendWorldSnapshot(connId, netId->GetNetId());

	Online::Log::Error("Player joined world: " + req.playerName + ", connId: " + std::to_string(connId));

#endif
}

void Online::Game::GameWorld::HandleExitWorldNotice(int connId)
{
#ifdef PIXEL_SERVER
	if (!serverWorld)
		return;

	auto netView = serverWorld->ecsRegistry.view<NetID>();
	std::vector<uint32_t> destroyedNetIds;
	std::vector<entt::entity> entitiesToDestroy;

	for (auto entity : netView)
	{
		auto& netIdComp = netView.get<NetID>(entity);
		if (netIdComp.GetOwnerConnId() == connId)
		{
			entitiesToDestroy.push_back(entity);
			destroyedNetIds.push_back(netIdComp.GetNetId());
		}
	}

	if (entitiesToDestroy.empty())
	{
		Online::Log::Info("No entities found for disconnecting connId: " + std::to_string(connId));
		return;
	}

	for (entt::entity entity : entitiesToDestroy)
		serverWorld->DestroyEntity(entity);

	Net::EntityDestroyPacket destroyPkt;
	destroyPkt.netIds = std::move(destroyedNetIds);
	auto payload = destroyPkt.SerializePayload();

	Net::Server::BroadcastUnreliableExcept(
		connId,
		payload,
		Net::PacketType::EntityDestroy,
		Net::ChannelType::ReliableUnordered
	);

	Online::Log::Info("Player disconnected (connId=" + std::to_string(connId) +
		"), destroyed " + std::to_string(destroyPkt.netIds.size()) + " entities, broadcasted destroy packet.");
#endif
}

void Online::Game::GameWorld::SendWorldSnapshot(int connId, uint32_t localPlayerNetId)
{

#ifdef PIXEL_SERVER
	if (serverWorld)
		serverWorld->ProcessDelayDestroyQueue();

	std::vector<std::byte> payload;
	serverWorld->SerializeToBytes(payload,Serialize::API::Json);

	Online::Net::WorldSnapshot snapshot;
	snapshot.localPlayerNetId = localPlayerNetId;
	snapshot.sceneData = std::move(payload);
	std::vector<std::byte> netPayload = snapshot.SerializePayload();

	Net::Server::SendReliable(
		connId,
		netPayload,
		Net::PacketType::WorldSnapshot,
		Net::ChannelType::ReliableOrdered
	);

	Online::Log::Error(
		"Sent world snapshot to connId: " + std::to_string(connId) +
		", raw size: " + std::to_string(payload.size()) +
		", net packet size: " + std::to_string(netPayload.size()) + " bytes"
	); 
#endif
}

void Online::Game::GameWorld::HandleEntityDataRequest(int connId, const Online::Net::ReqEntityDataPacket& req)
{	
#ifdef PIXEL_SERVER
	Scene* scene = GetActiveScene();
	if (!scene) return;

	auto netView = scene->ecsRegistry.view<NetID>();
	entt::entity entity = entt::null;
	for (auto e : netView)
	{
		if (netView.get<NetID>(e).GetNetId() == req.targetNetId)
		{
			entity = e;
			break;
		}
	}
	if (entity == entt::null) return;

	Net::EntityFullData full;
	full.netId = req.targetNetId;

	auto* netIdComp = scene->GetComponent<NetID>(entity);
	if (netIdComp)
		full.ownerConnId = netIdComp->GetOwnerConnId();

	auto* tag = scene->GetComponent<Tag>(entity);
	if (tag)
	{
		full.entityName = tag->GetName();
		full.entityTag = tag->GetTag();
	}

	auto* trans = scene->GetComponent<Transform>(entity);
	if (trans)
	{
		full.position = trans->GetWorldPosition();
		full.scale = trans->GetWorldScale();
		full.rotation = trans->GetWorldRotation();
	}

	auto* rb = scene->GetComponent<Rigidbody>(entity);
	if (rb)
	{
		full.hasRigidbody = true;
		full.gravityScale = rb->getGravityScale();
		full.fixedRotation = rb->IsFixRotation();
		full.bodyType = rb->GetBodyType();
	}

	auto* col = scene->GetComponent<Collider>(entity);
	if (col)
	{
		full.hasCollider = true;
		full.shape = col->GetShape();
		full.halfSize = col->GetHalfSize();
		full.radius = col->GetRadius();
		full.density = col->GetDensity();
		full.friction = col->GetFriction();
		full.restitution = col->GetRestitution();
		full.layerBits = col->GetCategoryBits();
	}

	GameObject* go = scene->GetGameObject(entity);
	if (go)
	{
		for (auto id : go->GetScriptIDSet())
			full.scriptIds.push_back(static_cast<uint32_t>(id));

		if (go->HasScriptFunction(Script::ScriptFunctionID::MoveLeftRight))
		{
			full.hasCharacter = true;
			full.roleId = Game::RoleID::SilverHat;
		}
	}



	Net::RespEntityDataPacket resp;
	resp.entityData = std::move(full);
	auto payload = resp.SerializePayload();

	Net::Server::SendReliable(
		connId,
		payload,
		Net::PacketType::RespEntityData,
		Net::ChannelType::ReliableOrdered
	);

	Online::Log::Info("Sent EntityFullData for netId=" + std::to_string(req.targetNetId) +
		" to connId=" + std::to_string(connId));
#endif
}

void Online::Game::GameWorld::ResetLoadingSceneState()
{
	if (!loadingScene) return;
}

void Online::Game::GameWorld::ApplyLoadedScene(Scene* newScene)
{
	if (showLoadingScene && loadingScene)
	{
		showLoadingScene = false;
	}	

	if (!newScene)
	{
		activeScene = nullptr;
		Log::Error("ApplyLoadedScene: newScene is null");
		return;
	}
	UnloadCurrentScene();
	activeScene = newScene;
	Log::Info("GameWorld: Scene switched successfully!");

#ifdef PIXEL_CLIENT
	GameObject* localPlayer = GetLocalPlayer();

	if (localPlayer == nullptr)
	{
		Log::Warning("未找到本地玩家，摄像机不绑定跟随");
		return;
	}
#endif
}