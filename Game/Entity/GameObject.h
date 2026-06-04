#pragma once
#include<Core/ObjectPool/ObjectPool.h>
#include<Core/Allocate/Allocate.h>
#include<Script/Common/ScriptFunctionID.h>
#include<Game/Common/FuncTable.h>
#include<Game/Component/Component.h>
#include<Game/Component/Tag.h>
#include<Game/Component/Parent.h>
#include<Game/Component/ChildLink.h>
#include<Game/Component/Transform.h>
#include<Game/Component/Animator.h>
#include<Game/Component/Sprite.h>
#include<Game/Component/Camera.h>
#include<Game/Component/ProgressBar.h>
#include<Game/Component/Follow.h>
#include<Game/Component/AudioListener.h>
#include<Game/Component/AudioSource.h>
#include<Game/Component/Collider.h>
#include<Game/Component/Rigidbody.h>
#include<Game/Component/ColliderList.h>
#include<Game/Component/TileMap.h>
#include<Game/Component/AnimatorController.h>

#include<Script/Common/FuncTable.h>

#include <entt/entt.hpp>
#include <string_view>
#include <vector>

namespace Online::Game
{
    class GameObject
    {
        friend class Online::Game::Scene;
        friend class Online::Core::ObjectPool<GameObject>;
    private:
        GameObject() :Entity(entt::null) {}
        ~GameObject() = default;

        inline void Reset(entt::entity entity) noexcept
        {
            Entity = entity;
            ActiveSelf = true;
            ActiveInHierarchy = true;
            scriptData.resize(static_cast<size_t>(Script::ScriptFunctionID::Count), nullptr);
            activeScripts.clear();
        }

        inline void OnActiveChanged(bool newActiveInHierarchy) noexcept
        {
            ActiveInHierarchy = newActiveInHierarchy;

            if (newActiveInHierarchy)
            {
                if (auto* transform = GetComponent<Transform>()) transform->OnEnable();
                if (auto* sprite = GetComponent<Sprite>()) sprite->OnEnable();
                if (auto* animator = GetComponent<Animator>()) animator->OnEnable();
                if (auto* camera = GetComponent<Camera>()) camera->OnEnable();
                if (auto* progressBar = GetComponent<ProgressBar>()) progressBar->OnEnable();
            }
            else
            {
                if (auto* progressBar = GetComponent<ProgressBar>()) progressBar->OnDisable();
                if (auto* camera = GetComponent<Camera>()) camera->OnDisable();
                if (auto* animator = GetComponent<Animator>()) animator->OnDisable();
                if (auto* sprite = GetComponent<Sprite>()) sprite->OnDisable();
                if (auto* transform = GetComponent<Transform>()) transform->OnDisable();
            }

            std::vector<GameObject*> children = GetChildren();
            for (GameObject* child : children)
            {
                bool childNewActive = newActiveInHierarchy && child->IsActiveSelf();
                if (childNewActive != child->IsActive())
                {
                    child->OnActiveChanged(childNewActive);
                }
            }

            for (auto id : activeScripts) 
            {
                auto* info = Script::GetInfo(id);
                if (!info) continue;
                if (newActiveInHierarchy && info->onEnable)
                    info->onEnable(this);
                else if (!newActiveInHierarchy && info->onDisable)
                    info->onDisable(this);
            }
        }

    public:
        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = delete;
        GameObject& operator=(GameObject&&) = delete;

    public:
        void Update(float deltaTime)
        {
            if (this->GetName() == "dog")
            {
            }
        }

        void ExecuteScriptUpdate(float dt) 
        {
            for (auto id : activeScripts) 
            {
                auto* info = Script::GetInfo(id);
                if (info && info->onUpdate)
                    info->onUpdate(this, dt);
            }
        }

        void ExecuteScriptLateUpdate(float dt) 
        {
            for (auto id : activeScripts) 
            {
                auto* info = Script::GetInfo(id);
                if (info && info->onLateUpdate)
                    info->onLateUpdate(this, dt);
            }
        }

        void ExecuteScriptFixedUpdate() 
        {
            for (auto id : activeScripts) 
            {
                auto* info = Script::GetInfo(id);
                if (info && info->onFixedUpdate)
                    info->onFixedUpdate(this);
            }
        }

        void AddScriptFunction(Script::ScriptFunctionID id) 
        {
            auto* info = Script::GetInfo(id);
            if (!info || info->dataSize == 0) return;

            if (scriptData[static_cast<size_t>(id)] != nullptr)
                RemoveScriptFunction(id);

            void* data = Core::Allocate(info->dataSize);
            if (info->constructor) info->constructor(data);
            scriptData[static_cast<size_t>(id)] = data;
            activeScripts.push_back(id);

            if (IsActive() && info->onEnable)
                info->onEnable(this);
        }

        void RemoveScriptFunction(Script::ScriptFunctionID id) 
        {
            auto idx = static_cast<size_t>(id);
            if (scriptData[idx] == nullptr) return;

            auto* info = Script::GetInfo(id);
            if (IsActive() && info && info->onDisable)
                info->onDisable(this);

            if (info && info->destructor) info->destructor(scriptData[idx]);
            Core::Free(scriptData[idx]);
            scriptData[idx] = nullptr;

            auto it = std::find(activeScripts.begin(), activeScripts.end(), id);
            if (it != activeScripts.end()) activeScripts.erase(it);
        }

        std::vector<Script::ScriptFunctionID> GetScriptIDSet()
        {
            return activeScripts;
        }
        
        template<typename T>
        T* GetScriptData(Script::ScriptFunctionID id) 
        {
            auto idx = static_cast<size_t>(id);
            return static_cast<T*>(scriptData[idx]);
        }
    public:
        inline entt::entity GetEntity() const noexcept
        {
            return Entity;
        }

        inline bool IsActive() const noexcept
        {
            return ActiveInHierarchy;
        }

        inline bool IsActiveSelf() const noexcept
        {
            return ActiveSelf;
        }

        inline void SetActive(bool active) noexcept
        {
            if (ActiveSelf == active)
                return;

            ActiveSelf = active;

            GameObject* parent = GetParent();
            bool newActiveInHierarchy = parent
                ? (parent->IsActive() && active)
                : active;

            if (newActiveInHierarchy == ActiveInHierarchy)
                return;

            OnActiveChanged(newActiveInHierarchy);
        }

        inline std::string_view GetName() const noexcept
        {
            if (auto* tag = GetComponent<Tag>()) {
                return tag->GetName();
            }
            return "Unnamed";
        }

        inline std::string_view GetTag() const noexcept
        {
            if (auto* tag = GetComponent<Tag>()) {
                return tag->GetTag();
            }
            return "default";
        }

        inline void SetName(std::string_view name) noexcept
        {
            if (auto* tag = GetComponent<Tag>())
            {
                tag->SetName(name);
            }
        }

        inline void SetTag(std::string_view tag) noexcept
        {
            if (auto* tagComp = GetComponent<Tag>()) {
                tagComp->SetTag(tag);
            }
        }

        inline Transform* GetTransform() noexcept
        {
            return GetComponent<Transform>();
        }

        inline const Transform* GetTransform() const noexcept
        {
            return GetComponent<Transform>();
        }

        inline GameObject* GetParent() const noexcept
        {
            if (auto* parent = GetComponent<Parent>())
            {
                return Online::Game::GetGameObject(parent->GetId());
            }
            return nullptr;
        }

        inline void SetParent(GameObject* parent, bool keepWorldTransform = true)
        {
            entt::entity parentId = parent ? parent->GetEntity() : entt::null;
            Online::Game::SetRelationship(Entity, parentId, entt::null, keepWorldTransform);

            bool newActiveInHierarchy = parent
                ? (parent->IsActive() && ActiveSelf)
                : ActiveSelf;

            if (newActiveInHierarchy != ActiveInHierarchy)
            {
                OnActiveChanged(newActiveInHierarchy);
            }
        }

        inline std::vector<GameObject*> GetChildren() const noexcept
        {
            std::vector<GameObject*> children;
            if (auto* link = GetComponent<ChildLink>())
            {
                entt::entity currChild = link->GetFirstChild();
                while (currChild != entt::null) {
                    GameObject* childObj = GetGameObject(currChild);
                    if (childObj) {
                        children.push_back(childObj);
                        ChildLink* childLink = childObj->GetComponent<ChildLink>();
                        currChild = childLink ? childLink->GetNextSibling() : entt::null;
                    }
                    else {
                        currChild = entt::null;
                    }
                }
            }
            return children;
        }

        inline GameObject* GetChild(size_t idx) const noexcept
        {
            if (auto* link = GetComponent<ChildLink>())
            {
                entt::entity currChild = link->GetFirstChild();
                size_t currentIdx = 0;
                while (currChild != entt::null) {
                    if (currentIdx == idx) {
                        return GetGameObject(currChild);
                    }
                    GameObject* childObj = GetGameObject(currChild);
                    ChildLink* childLink = childObj ? childObj->GetComponent<ChildLink>() : nullptr;
                    currChild = childLink ? childLink->GetNextSibling() : entt::null;
                    currentIdx++;
                }
            }
            return nullptr;
        }

        inline GameObject* FindChild(std::string_view name) const noexcept
        {
            if (auto* link = GetComponent<ChildLink>()) {
                entt::entity currChild = link->GetFirstChild();
                while (currChild != entt::null) {
                    GameObject* child = GetGameObject(currChild);
                    if (child && child->GetName() == name) {
                        return child;
                    }

                    GameObject* childObj = GetGameObject(currChild);
                    ChildLink* childLink = childObj ? childObj->GetComponent<ChildLink>() : nullptr;
                    currChild = childLink ? childLink->GetNextSibling() : entt::null;
                }
            }
            return nullptr;
        }

        inline void Destroy() noexcept
        {
            Online::Game::DestroyEntity(Entity);
        }

        inline bool IsValid() const noexcept
        {
            return Online::Game::GetGameObject(Entity) == this;
        }

    public:
        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of_v<Component, T>, "All components must inherit from Component");

            auto& registry = Online::Game::GetRegistry();
            T& component = registry.emplace_or_replace<T>(Entity, std::forward<Args>(args)...);

            component.gameObject = this;

            if (IsActive())
            {
                component.OnEnable();
            }

            return component;
        }

        template<typename T>
        T* GetComponent() noexcept
        {
            auto& registry = Online::Game::GetRegistry();
            return registry.try_get<T>(Entity);
        }

        template<typename T>
        const T* GetComponent() const noexcept
        {
            auto& registry = Online::Game::GetRegistry();
            return registry.try_get<T>(Entity);
        }

        template<typename T>
        bool HasComponent() const noexcept
        {
            auto& registry = Online::Game::GetRegistry();
            return registry.all_of<T>(Entity);
        }

        template<typename T>
        void RemoveComponent() noexcept
        {
            auto* component = GetComponent<T>();
            if (component)
            {
                if (IsActive())
                {
                    component->OnDisable();
                }

                auto& registry = Online::Game::GetRegistry();
                registry.remove<T>(Entity);
            }
        }

        template<typename T, typename... Args>
        T& GetOrAddComponent(Args&&... args)
        {
            if (auto* comp = GetComponent<T>()) {
                return *comp;
            }
            return AddComponent<T>(std::forward<Args>(args)...);
        }

        template<typename T>
        bool TryGetComponent(T*& outComponent) noexcept
        {
            outComponent = GetComponent<T>();
            return outComponent != nullptr;
        }

        template<typename T>
        T* TryGetComponent() noexcept
        {
            return GetComponent<T>();
        }

    private:
        entt::entity Entity = entt::null;
        bool ActiveSelf = true;
        bool ActiveInHierarchy = true;

        std::vector<void*> scriptData;
        std::vector<Script::ScriptFunctionID> activeScripts;
    };
}