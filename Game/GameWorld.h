#pragma once
#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>
#include <Game/Scene/Scene.h>

#include<entt/entt.hpp>

namespace Online::Game
{
    class Scene;
    class GameWorld
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<GameWorld>;
        private:
            static GameWorld* Create()
            {
                return ONLINE_NEW(GameWorld);
            }
            static void Destroy(GameWorld* gameworld) 
            {
                ONLINE_DELETE(gameworld); 
            }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<GameWorld>;
        private:
            static bool Initialize(GameWorld* gameWorld)
            {
                return gameWorld->Initialize();
            }
            static void Release(GameWorld* gameWorld)
            {
                gameWorld->Release();
            }
            static void FixedUpdate(GameWorld* gameWorld)
            {
				gameWorld->FixedUpdate();   
			}
            static void Update(GameWorld* gameWorld)
            {
                gameWorld->Update();
            }
            static void LateUpdate(GameWorld* gameWorld)
            {
                gameWorld->LateUpdate();
            }
            static void EndFrame(GameWorld* gameWorld)
            {
                gameWorld->EndFrame();
            }
        };

    private:
        GameWorld() = default;
        ~GameWorld() = default;

    public:
        GameWorld(const GameWorld&) = delete;
        GameWorld& operator=(const GameWorld&) = delete;
        GameWorld(GameWorld&&) = delete;
        GameWorld& operator=(GameWorld&&) = delete;

    private:
        bool Initialize();
        void Release();
        void FixedUpdate();
        void Update();
        void LateUpdate();
        void EndFrame();         
    public:
        void LoadScene(const std::string& sceneName);
        void SwitchSceneAfterLoadingAsync(const std::string& newSceneName);
        void SwitchSceneAsync(const std::string& newSceneName);
        bool IsSceneLoading() const { return isAsyncLoading; }
        void DisplayPendingScene();
        bool IsPendingSceneReady() { return pendingScene != nullptr; }
    private:
        void ResetLoadingSceneState();
        void ApplyLoadedScene(Scene* newScene);
        void UnloadCurrentScene();

    public:
        inline Scene* GetActiveScene() const noexcept
        {
            return activeScene;
        }
        inline entt::registry& GetRegistry() noexcept
        {
            return activeScene->ecsRegistry;
        }
        inline void TransformUpdater(entt::entity entity, glm::vec2 trans, float angle) noexcept
        {
            if (!activeScene)
                return;

			activeScene->TransformUpdater(entity, trans, angle);
        }
        void TransformUpdater(uint32_t entity, glm::vec2 trans, float angle) noexcept
        {
            if (!activeScene)
                return;

			activeScene->TransformUpdater(static_cast<entt::entity>(entity), trans, angle);
        }
        inline GameObject* GetGameObject(entt::entity entity) noexcept
        {
            if (!activeScene)
                return nullptr;
            return activeScene->GetGameObject(entity);
        }
        inline void DestroyEntity(entt::entity entity) noexcept
        {
            if (!activeScene)
                return;
            activeScene->DestroyEntity(entity);
        }
        inline void SetRelationship(entt::entity childId, entt::entity parentId, entt::entity afterSibling = entt::null, bool keepWorldTransform = true)
        {
            if (!activeScene)
                return;
			activeScene->SetRelationship(childId, parentId, afterSibling, keepWorldTransform);
        }
        inline glm::vec2 GetWorldPosition(entt::entity entity) noexcept
        {
            if (!activeScene)
                return glm::vec2(0.0f);
            auto* transform = activeScene->GetComponent<Transform>(entity);
            if (!transform)
                return glm::vec2(0.0f);
            return transform->GetWorldPosition();
		}
        inline float GetWorldRotation(entt::entity entity) noexcept
        {
            if (!activeScene)
                return 0.0f;
            auto* transform = activeScene->GetComponent<Transform>(entity);
            if (!transform)
                return 0.0f;
            return transform->GetWorldRotation();
		}

    public:

    private:
        Scene* activeScene = nullptr;
        Scene* loadingScene = nullptr;
        Scene* pendingScene = nullptr;

        std::unordered_map<std::string, std::string> BuildSettings;
        bool isAsyncLoading = false;
        bool showLoadingScene = false;
        bool pendingApplyScene = false;
    };
}