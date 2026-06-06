#include<Core/Utils/File.h>
#include<Game/GameWorld.h>
#include<Game/Scene/Scene.h>
#include<Game/Entity/GameObject.h>  
#include<Time/Common/FuncTable.h>
#include<Input/Common/FuncTable.h>
#include<Task/Common/FuncTable.h>

#include<chrono>
#include<thread> 

bool Online::Game::GameWorld::Initialize()
{
	BuildSettings["BeginScene"] = Core::GetExeDir() + "scenes\\begin.json";
	BuildSettings["LoadingScene"] = Core::GetExeDir() + "scenes\\loading.json";
	BuildSettings["TestScene"] = Core::GetExeDir() + "scenes\\test.json";

	//loadingScene = ONLINE_NEW(Scene);
	//if (!loadingScene->DeserializeFromFile(BuildSettings["LoadingScene"], Serialize::API::Json))
	//{
	//	ONLINE_DELETE(loadingScene);
	//	loadingScene = nullptr;
	//	throw std::runtime_error("Failed to load loading scene");
	//}

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
	mainCamera->GetTransform()->SetLocalPosition({ 0.0f, 0.0f });

	AudioListener& listener = mainCamera->AddComponent<AudioListener>();
	listener.SetMusic(Asset::MusicID::Mus_BackGround);
	listener.SetMusicVolume(0.3f);

	GameObject* back = activeScene->CreateGameObject("back");
	back->GetComponent<Transform>()->SetLocalScale({ 4,4 });

	GameObject* background = activeScene->CreateGameObject();
	Sprite& backSprite = background->AddComponent<Sprite>();
	backSprite.SetAnchor(Core::Anchor::TopLeft);
	backSprite.SetTexture(Asset::TextureID::Tex_BackGround_Far);
	backSprite.SetRenderQueue(Render::RenderQueue::Background);
	backSprite.SetDepth(2);

	GameObject* midground = activeScene->CreateGameObject();
	Sprite& midSprite = midground->AddComponent<Sprite>();
	midSprite.SetAnchor(Core::Anchor::TopLeft);
	midSprite.SetTexture(Asset::TextureID::Tex_BackGround_Mid);
	midSprite.SetRenderQueue(Render::RenderQueue::Background);
	midSprite.SetDepth(3);

	GameObject* frontground = activeScene->CreateGameObject();
	Sprite& frontSprite = frontground->AddComponent<Sprite>();
	frontSprite.SetAnchor(Core::Anchor::TopLeft);
	frontSprite.SetTexture(Asset::TextureID::Tex_BackGround_Near);
	frontSprite.SetRenderQueue(Render::RenderQueue::Background);
	frontSprite.SetDepth(4);

	background->SetParent(back, false);
	midground->SetParent(back, false);
	frontground->SetParent(back, false);

	GameObject* drink = activeScene->CreateGameObject("drink");
	Transform* drinkTrans = drink->GetComponent<Transform>();
	drinkTrans->SetLocalPosition({ 620, 310 });
	drinkTrans->SetLocalScale({ 4,4 });
	Sprite& drinkSprite = drink->AddComponent<Sprite>();
	drinkSprite.SetAnchor(Core::Anchor::Center);
	drinkSprite.SetColor(Core::Color::Black);
	Animator& drinkAnim = drink->AddComponent<Animator>();

	drinkAnim.SetSprite(&drinkSprite);
	drinkAnim.Play(Asset::AnimationClipID::Anim_Loading);

	GameObject* text = activeScene->CreateGameObject("text");
	Transform* textTrans = text->GetComponent<Transform>();
	textTrans->SetLocalPosition({ 560, 450 });
	textTrans->SetLocalScale({ 1.5,1.5 });
	Text& textComp = text->AddComponent<Text>();
	textComp.SetAnchor(Core::Anchor::TopLeft);
	textComp.SetColor(Core::Color::Black);
	textComp.SetFont(Asset::FontID::Deng);
	textComp.SetRenderQueue(Render::RenderQueue::Background);
	textComp.SetDepth(5);
	//textComp.SetLetterSpacing(-3);
	text->AddScriptFunction(Script::ScriptFunctionID::LoadingScene);

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
	if (activeScene)
		activeScene->LateUpdate(Time::delta());

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