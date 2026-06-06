#pragma once
#include <Context/Context.h>
#include <Client/Context/ClientContext.h>

#include <entt/entt.hpp>
#include <glm.hpp>

namespace Online::Game {
	class Scene;
	class GameObject;
}

namespace Online::Runtime
{
	template<>
	struct FuncTable<Online::Game::GameWorld>
	{
		friend class Online::Runtime::Client;
	private:
		FuncTable() = default;
		~FuncTable() = default;

	public:
		FuncTable(const FuncTable&) = delete;
		FuncTable& operator=(const FuncTable&) = delete;
		FuncTable(FuncTable&&) = delete;
		FuncTable& operator=(FuncTable&&) = delete;

	public:
		inline bool Check() const
		{
			if (!OnGetActiveScene) { throw std::runtime_error("Delegates miss [GameWorld::GetActiveScene] Function!"); }
			if (!OnGetRegistry) { throw std::runtime_error("Delegates miss [GameWorld::GetRegistry] Function!"); }
			if (!OnDestroyEntity) { throw std::runtime_error("Delegates miss [GameWorld::DestoryEntity] Function!"); }
			if (!OnGetGameObject) { throw std::runtime_error("Delegates miss [GameWorld::OnGetGameObject] Function!"); }
			if (!OnSetRelationship) { throw std::runtime_error("Delegates miss [GameWorld::OnSetRelationship] Function!"); }
			if (!OnTransformUpdater) { throw std::runtime_error("Delegates miss [GameWorld::OnTransformUpdater] Function!"); }
			if (!OnGetWorldPosition) { throw std::runtime_error("Delegates miss [GameWorld::OnGetWorldPosition] Function!"); }
			if (!OnGetWorldRotation) { throw std::runtime_error("Delegates miss [GameWorld::OnGetWorldRotation] Function!"); }
			if (!OnSwitchSceneAfterLoadingAsync) { throw std::runtime_error("Delegates miss [GameWorld::SwitchSceneAfterLoadingAsync] Function!"); }
			if (!OnSwitchSceneAsync) { throw std::runtime_error("Delegates miss [GameWorld::SwitchSceneAsync] Function!"); }
			if (!OnLoadScene) { throw std::runtime_error("Delegates miss [GameWorld::LoadScene] Function!"); }
			if (!OnIsSceneLoading) { throw std::runtime_error("Delegates miss [GameWorld::IsSceneLoading] Function!"); }
			if (!OnDisplayPendingScene) { throw std::runtime_error("Delegates miss [GameWorld::DisplayPendingScene] Function!"); }
			if (!OnIsPendingSceneReady) { throw std::runtime_error("Delegates miss [GameWorld::IsPendingSceneReady] Function!"); }
			return true;
		}
		inline void UnRegister() noexcept
		{
			OnGetActiveScene = nullptr;
			OnGetRegistry = nullptr;
			OnDestroyEntity = nullptr;
			OnGetGameObject = nullptr;
			OnSetRelationship = nullptr;
			OnTransformUpdater = nullptr;
			OnGetWorldPosition = nullptr;
			OnGetWorldRotation = nullptr;
			OnSwitchSceneAfterLoadingAsync = nullptr;
			OnSwitchSceneAsync = nullptr;
			OnLoadScene = nullptr;
			OnIsSceneLoading = nullptr;
			OnDisplayPendingScene = nullptr;
			OnIsPendingSceneReady = nullptr;
		}
	public:
		inline Online::Game::Scene* InvokeOnGetActiveScene() const noexcept
		{
			return OnGetActiveScene();
		}
		inline entt::registry& InvokeOnGetRegistry() const noexcept
		{
			return OnGetRegistry();
		}
		inline void InvokeOnDestoryEntity(entt::entity entity) noexcept
		{
			OnDestroyEntity(entity);
		}
		inline Online::Game::GameObject* InvokeOnGetGameObject(entt::entity entity) noexcept
		{
			return OnGetGameObject(entity);
		}
		inline void InvokeOnSetRelationship(entt::entity childId, entt::entity parentId, entt::entity afterSibling, bool keepWorldTransform) noexcept
		{
			OnSetRelationship(childId, parentId, afterSibling, keepWorldTransform);
		}
		inline void InvokeTransformUpdater(entt::entity entity, glm::vec2 trans, float angle) noexcept
		{
			OnTransformUpdater(entity, trans, angle);
		}
		inline glm::vec2 InvokeOnGetWorldPosition(entt::entity entity) const noexcept
		{
			return OnGetWorldPosition(entity);
		}
		inline float InvokeOnGetWorldRotation(entt::entity entity) const noexcept
		{
			return OnGetWorldRotation(entity);
		}
		inline void InvokeOnSwitchSceneAfterLoadingAsync(const std::string& newSceneName) noexcept
		{
			OnSwitchSceneAfterLoadingAsync(newSceneName);
		}
		inline void InvokeOnSwitchSceneAsync(const std::string& newSceneName) noexcept
		{
			OnSwitchSceneAsync(newSceneName);
		}
		inline void InvokeOnLoadScene(const std::string& sceneName) noexcept
		{
			OnLoadScene(sceneName);
		}
		inline bool InvokeOnIsSceneLoading() const noexcept
		{
			return OnIsSceneLoading();
		}
		inline void InvokeOnDisplayPendingScene() noexcept
		{
			OnDisplayPendingScene();
		}
		inline bool InvokeOnIsPendingSceneReady() const noexcept
		{
			return OnIsPendingSceneReady();
		}
	private:
		Online::Game::Scene* (*OnGetActiveScene)() noexcept = nullptr;
		entt::registry& (*OnGetRegistry)() noexcept = nullptr;
		void (*OnDestroyEntity)(entt::entity) noexcept = nullptr;
		Online::Game::GameObject* (*OnGetGameObject)(entt::entity) noexcept = nullptr;
		void (*OnSetRelationship)(entt::entity, entt::entity, entt::entity, bool) noexcept = nullptr;
		void (*OnTransformUpdater)(entt::entity, glm::vec2, float) noexcept = nullptr;
		glm::vec2(*OnGetWorldPosition)(entt::entity) noexcept = nullptr;
		float (*OnGetWorldRotation)(entt::entity) noexcept = nullptr;
		void (*OnSwitchSceneAfterLoadingAsync)(const std::string& newSceneName) noexcept = nullptr;
		void (*OnSwitchSceneAsync)(const std::string& newSceneName) noexcept = nullptr;
		void (*OnLoadScene)(const std::string& sceneName) noexcept = nullptr;
		bool (*OnIsSceneLoading)() noexcept = nullptr;
		void (*OnDisplayPendingScene)() noexcept = nullptr;
		bool (*OnIsPendingSceneReady)() noexcept = nullptr;
	};
}

namespace Online::Game
{
	inline Online::Game::Scene* GetActiveScene() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnGetActiveScene();
	}
	inline entt::registry& GetRegistry() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnGetRegistry();
	}
	inline void DestroyEntity(entt::entity entity) noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnDestoryEntity(entity);
	}
	inline Online::Game::GameObject* GetGameObject(entt::entity entity) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnGetGameObject(entity);
	}
	inline void SetRelationship(entt::entity childId, entt::entity parentId, entt::entity afterSibling, bool keepWorldTransform) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnSetRelationship(childId, parentId, afterSibling, keepWorldTransform);
	}
	inline void TransformUpdater(entt::entity entity, glm::vec2 trans, float angle) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeTransformUpdater(entity, trans, angle);
	}
	inline glm::vec2 GetWorldPosition(entt::entity entity) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnGetWorldPosition(entity);
	}
	inline float GetWorldRotation(entt::entity entity) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnGetWorldRotation(entity);
	}
	inline void SwitchSceneAfterLoadingAsync(const std::string& newSceneName) noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnSwitchSceneAfterLoadingAsync(newSceneName);
	}
	inline void SwitchSceneAsync(const std::string& newSceneName) noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnSwitchSceneAsync(newSceneName);
	}
	inline void LoadScene(const std::string& sceneName) noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnLoadScene(sceneName);
	}
	inline bool IsSceneLoading() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnIsSceneLoading();
	}
	inline void DisplayPendingScene() noexcept
	{
		Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnDisplayPendingScene();
	}
	inline bool IsPendingSceneReady() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Game::GameWorld>().InvokeOnIsPendingSceneReady();
	}
}