#pragma once

#include<Core/Allocate/Allocate.h>
#include<Core/ObjectPool/ObjectPool.h>
#include<Serialize/Serializable.h>
#include<Event/Common/EventToken.h>
#include<Event/Common/Event.h>
#include<Game/Component/Component.h>
#include<Game/Component/Tag.h>
#include<Game/Component/Parent.h>
#include<Game/Component/ChildLink.h>
#include<Game/Component/Transform.h>
#include<Game/Component/Animator.h>
#include<Game/Component/Sprite.h>
#include<Game/Component/Camera.h>
#include<Game/Component/Collider.h>
#include<Game/Component/Rigidbody.h>

#include<entt/entt.hpp>
#include<glm.hpp>
#include<unordered_map>

namespace Online::Game
{
    class GameWorld;
    class Scene : public Online::Serialize::Serializable
    {
        friend class GameWorld;
        friend class GameObject;
    private:
        Scene();
        ~Scene();

    public:
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

    private:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override;
        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override;

    public:
        entt::entity GetRootEntity() const noexcept;
        GameObject* GetRootGameObject() const noexcept;

    public:
        void ProcessTransformDirtySystem();

        GameObject* CreateGameObject(std::string_view name = "GameObject", std::string_view tag = "default");
        void DestroyEntity(entt::entity entity);
        void SetRelationship(entt::entity childId, entt::entity parentId, entt::entity afterSibling = entt::null, bool keepWorldTransform = true);
        inline void TransformUpdater(entt::entity entity, glm::vec2 trans, float angle) noexcept
        {
            if (entity == entt::null)
                return;

            auto* transform = this->ecsRegistry.try_get<Transform>(entity);
            if (!transform)
                return;

            transform->SetWorldPosition(trans);
            transform->SetWorldRotation(angle);
        }
        GameObject* GetGameObject(entt::entity entity) noexcept;
        const GameObject* GetGameObject(entt::entity entity) const noexcept;

        template<typename... Components>
        auto GetView()
        {
            return ecsRegistry.view<Components...>();
        }

        template<typename... Components, typename... Excludables>
        auto GetView(entt::exclude_t<Excludables...> exclude)
        {
            return ecsRegistry.view<Components...>(exclude);
        }
    private:
        void Update(float deltaTime);
        void LateUpdate(float deltaTime);
    private:
        entt::entity CreateEntity(std::string_view name = "GameObject", std::string_view tag = "default");
        void ProcessDelayDestroyQueue();

        void ProcessAnimationSystem(float deltaTime);
        void ProcessProgressBarSystem(float deltaTime);
        void ProcessFollowSystem(float deltaTime);
        void ProcessColliderSystem();
        void ProcessColliderListSystem();
        void ProcessAnimatorControllerSystem(float deltaTime);

        bool IsDescendant(entt::entity maybeChild, entt::entity maybeParent);
        void UnlinkFromCurrent(entt::entity entity);
        void InsertIntoLinkList(entt::entity child, entt::entity parent, entt::entity after);
        void InsertIntoRootList(entt::entity child, entt::entity after);

    private:
        void OnTriggerEnter(const Online::Event::Event& event);
        void OnTriggerExit(const Online::Event::Event& event);
        void OnTriggerStay(const Online::Event::Event& event);
        void OnPhysFixedUpdate(const Online::Event::Event& event);

        static void OnTriggerEnterThunk(void* listener, const Online::Event::Event& event);
        static void OnTriggerExitThunk(void* listener, const Online::Event::Event& event);
        static void OnTriggerStayThunk(void* listener, const Online::Event::Event& event);
        static void OnFixedUpdateThunk(void* listener, const Online::Event::Event& event);

    public:
        template<typename T>
        T* GetComponent(entt::entity entity)
        {
            return ecsRegistry.try_get<T>(entity);
        }
        template<typename T>
        bool HasComponent(entt::entity entity) const
        {
            return ecsRegistry.all_of<T>(entity);
        }
        template<typename T>
        void RemoveComponent(entt::entity entity)
        {
            ecsRegistry.remove<T>(entity);
        }

    private:
        void MarkColliderTreeDirty(entt::entity rootEntity);

    private:
        entt::entity root = entt::null;
        entt::registry ecsRegistry;
        std::vector<entt::entity> delayDestroyQueue;
        Online::Core::ObjectPool<GameObject> gameObjectPool;
        std::unordered_map<entt::entity, GameObject*> entityToGameObject;
        std::unordered_map<std::string, GameObject*> gameObjects;

        Online::Event::EventToken triggerEnterToken;
        Online::Event::EventToken triggerExitToken;
        Online::Event::EventToken triggerStayToken;
        Online::Event::EventToken FixedUpdateToken;
    };
}