#include<Core/Utils/File.h>
#include<Game/GameWorld.h>
#include<Game/Scene/Scene.h>
#include<Game/Entity/GameObject.h>  
#include<Time/Common/FuncTable.h>
#include<Input/Common/FuncTable.h>
#include<Task/Common/FuncTable.h>

#include<chrono>
#include<thread> 

#include<Script/LifeCycleFunc/Follow.h>
#include<Script/LifeCycleFunc/Button.h>
#include<Script/LifeCycleFunc/TextInput.h>

bool Online::Game::GameWorld::Initialize()
{
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

	//if (activeScene)
	//{
	//	bool ok = activeScene->DeserializeFromFile(BuildSettings["BeginScene"], Online::Serialize::API::Json);
	//	if (!ok)
	//		Log::Error("Failed to load begin scene");
	//}

	GameObject* mainCamera = activeScene->CreateGameObject("Main Camera", "MainCamera");
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
	butone->AddScriptFunction(Script::ScriptFunctionID::Button);

	auto* buttonData = butone->GetScriptData<Script::Button::ButtonData>(Script::ScriptFunctionID::Button);
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

	GameObject* UICanvas = activeScene->CreateGameObject("Canvas");
	Sprite& uisprite = UICanvas->AddComponent<Sprite>();
	uisprite.SetTexture(Asset::TextureID::Tex_BackBuffer_1);
	uisprite.SetAnchor(Core::Anchor::TopLeft);
	UICanvas->AddScriptFunction(Script::ScriptFunctionID::FollowOverTime);
	UICanvas->GetScriptData<Script::Follow::FollowData>(Script::ScriptFunctionID::FollowOverTime)->SetTarget(mainCamera->GetComponent<Transform>());
	return true;
}

void Online::Game::GameWorld::Release()
{
	UnloadCurrentScene();
	if (loadingScene)
	{
		loadingScene = nullptr;
	}
}

void Online::Game::GameWorld::FixedUpdate()
{
	if (activeScene)
	{
		activeScene->ProcessTransformDirtySystem();
		activeScene->ProcessColliderSystem();
		activeScene->ProcessColliderListSystem();
	}
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

	if (activeScene)
	{
		activeScene->ProcessFollowSystem(Online::Time::delta());
		activeScene->ProcessAnimationSystem(Online::Time::delta());
		activeScene->ProcessAnimatorControllerSystem(Online::Time::delta());
		activeScene->ProcessProgressBarSystem(Online::Time::delta());
	}

	if (activeScene)
		activeScene->LateUpdate(Time::delta());

}

void Online::Game::GameWorld::EndFrame()
{
	if(Input::GetKeyDown(Input::KeyCode::P))
	{
		SwitchSceneAfterLoadingAsync("TestScene");
	}
	if (activeScene) { activeScene->ProcessDelayDestroyQueue(); }
}

void Online::Game::GameWorld::UnloadCurrentScene()
{
	if (activeScene == nullptr) return;

	activeScene->ProcessDelayDestroyQueue();
	Input::ResetAllState();

	ONLINE_DELETE(activeScene);
	activeScene = nullptr;

	if (activeScene != loadingScene)
	{
		ONLINE_DELETE(activeScene);
	}
	activeScene = nullptr;
	showLoadingScene = false;

	Online::Log::Info("GameWorld: Unloaded current scene successfully");
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

		isAsyncLoading = false;
		pendingScene = scene;

		}, "SceneAsyncLoad" + sceneName);

	return;
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

		pendingScene = scene;
		pendingApplyScene = true;

		}, "SceneAsyncLoad" + newSceneName);

}

void Online::Game::GameWorld::DisplayPendingScene()
{
	if (!pendingScene)
	{
		Log::Error("PendingScene is empty");
		return;
	}

	ApplyLoadedScene(pendingScene);
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
}