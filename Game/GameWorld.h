#pragma once
#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>
#include <Game/Scene/Scene.h>

#include <Net/Common/JoinWorldPacket.h>
#include <Net/Common/ReqEntityDataPacket.h>

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
        void LoadScene(const std::vector<std::byte>& sceneName);
        void SwitchSceneAfterLoadingAsync(const std::string& newSceneName);
        void SwitchSceneAsync(const std::string& newSceneName);
        bool IsSceneLoading() { return isAsyncLoading == true; }
        void DisplayPendingScene();
        bool IsPendingSceneReady() 
        {
            return pendingScene != nullptr; 
        }

    public:
        void SendJoinWorldRequest(const std::string& playerName, uint32_t playerId);

        void HandleJoinWorldRequest(int connId, const Online::Net::JoinWorldRequest& req);
        void SendWorldSnapshot(int connId, uint32_t localPlayerNetId);
        void HandleEntityDataRequest(int connId, const Online::Net::ReqEntityDataPacket& req);
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

        inline GameObject* FindGameObjectByName(std::string_view name) noexcept
        {
            if (!activeScene)
                return nullptr;
            return activeScene->FindGameObjectByName(name);
        }
        inline const GameObject* FindGameObjectByName(std::string_view name) const noexcept
        {
            if (!activeScene)
                return nullptr;
            return activeScene->FindGameObjectByName(name);
        }
        inline std::vector<GameObject*> FindGameObjectsAllByName(std::string_view name)
        {
            if (!activeScene)
                return {};
            return activeScene->FindGameObjectsAllByName(name);
        }
        inline std::vector<const GameObject*> FindGameObjectsAllByName(std::string_view name) const noexcept
        {
            if (!activeScene)
                return {};
            return static_cast<const Scene*>(activeScene)->FindGameObjectsAllByName(name);
        }

        inline GameObject* FindGameObjectByTag(std::string_view tagName) noexcept
        {
            if (!activeScene)
                return nullptr;
            return activeScene->FindGameObjectByTag(tagName);
        }
        inline const GameObject* FindGameObjectByTag(std::string_view tagName) const noexcept
        {
            if (!activeScene)
                return nullptr;
            return activeScene->FindGameObjectByTag(tagName);
        }
        inline std::vector<GameObject*> FindGameObjectsByTag(std::string_view tagName)
        {
            if (!activeScene)
                return {};
            return activeScene->FindGameObjectsByTag(tagName);
        }
        inline std::vector<const GameObject*> FindGameObjectsByTag(std::string_view tagName) const noexcept
        {
            if (!activeScene)
                return {};
            return static_cast<const Scene*>(activeScene)->FindGameObjectsByTag(tagName);
        }

        inline uint32_t Generate()
        {
            constexpr uint32_t MAX_NET_ID = UINT32_MAX - 1000;
            if (nextId >= MAX_NET_ID)
                nextId = 1;
            return ++nextId;
        }
        inline uint32_t GetServerFrame() const
        {
            return serverFrame;
		}

        inline uint32_t GetLocalPlayerNetId()
        {
            return localPlayerNetId;
        }
        inline GameObject* GetLocalPlayer()
        {
            if (!activeScene || localPlayerNetId == -1)
                return nullptr;

            auto& registry = activeScene->ecsRegistry;
            auto view = registry.view<NetID>();

            for (auto entity : view)
            {
                auto& netComp = view.get<NetID>(entity);
                if (netComp.GetNetId() == localPlayerNetId)
                {
                    return activeScene->GetGameObject(entity);
                }
            }
            return nullptr;
        }
    private:
        Scene* activeScene = nullptr;
        Scene* loadingScene = nullptr;
        Scene* pendingScene = nullptr;

		Scene* serverWorld = nullptr;

        std::unordered_map<std::string, std::string> BuildSettings;
        bool isAsyncLoading = false;
        bool showLoadingScene = false;
        bool pendingApplyScene = false;

#pragma region PIXEL_SERVER
        uint32_t nextId = 0;
        uint32_t serverFrame = 0;
        Online::Event::EventToken StepCompletedToken;
#pragma endregion

#pragma region PIXEL_CLIENT
        uint32_t localPlayerNetId = -1;
#pragma endregion

    };
}