#include<Event/Common/FuncTable.h>
#include<Event/Common/EventArgs.h>
#include<Game/Scene/Scene.h>
#include<Game/Entity/GameObject.h>
#include<Game/Component/Tag.h>
#include<Game/Common/EntityRefFixup.h>
#include<Game/Character/RoleID.h>
#include<Time/Common/FuncTable.h>

#ifdef PIXEL_CLIENT
#include<Net/Client/Common/FuncTable.h>
#include<Net/Common/ReqEntityDataPacket.h>
#include<Net/Common/EntityFullData.h>
#endif // PIXEL_CLIENT

#ifdef PIXEL_SERVER
#include<Net/Server/Common/FuncTable.h>
#include<Net/Common/ReqEntityDataPacket.h>
#endif // PIXEL_SERVER


#include<algorithm>

namespace Online::Game
{
    Scene::Scene()
        : gameObjectPool(nullptr, nullptr, 32)
        , CameraInited(false)
    {
        triggerEnterToken = Online::Event::Subscribe(
            Online::Event::EventType::PhysicsTriggerEnter, &Scene::OnTriggerEnterThunk, this);
        triggerExitToken = Online::Event::Subscribe(
            Online::Event::EventType::PhysicsTriggerExit, &Scene::OnTriggerExitThunk, this);
        triggerStayToken = Online::Event::Subscribe(
            Online::Event::EventType::PhysicsTriggerStay, &Scene::OnTriggerStayThunk, this);
        FixedUpdateToken = Online::Event::Subscribe(
            Online::Event::EventType::PhysFixedUpdate, &Scene::OnFixedUpdateThunk, this);
    }

    Scene::~Scene()
    {
        if (triggerEnterToken.type != Online::Event::EventType::Invalid)
            Online::Event::UnSubscribe(triggerEnterToken);
        if (triggerExitToken.type != Online::Event::EventType::Invalid)
            Online::Event::UnSubscribe(triggerExitToken);
        if (triggerStayToken.type != Online::Event::EventType::Invalid)
            Online::Event::UnSubscribe(triggerStayToken);
        if (FixedUpdateToken.type != Online::Event::EventType::Invalid)
            Online::Event::UnSubscribe(FixedUpdateToken);

        for (auto& pair : entityToGameObject)
        {
            gameObjectPool.Release(pair.second);
        }
        entityToGameObject.clear();
        ecsRegistry.clear();
        delayDestroyQueue.clear();
        root = entt::null;
    }

    void Scene::Serialize(Online::Serialize::SerializeContext& ctx) const
    {
        ctx.Write("version", 1);
        ctx.Write("root", static_cast<uint32_t>(root));

        ctx.BeginArray("entities");

        auto view = ecsRegistry.view<entt::entity>();
        for (auto entity : view)
        {
            Serialize::SerializeContext& entityCtx = ctx.WriteArrayObjectBegin();

            entityCtx.Write("id", static_cast<uint32_t>(entity));

            if (auto* tag = ecsRegistry.try_get<Tag>(entity))
            {
                tag->Serialize(entityCtx.GetSubContext("Tag"));
            }
            if (auto* trans = ecsRegistry.try_get<Transform>(entity))
            {
                trans->Serialize(entityCtx.GetSubContext("Transform"));
            }
            if (auto* sprite = ecsRegistry.try_get<Sprite>(entity))
            {
                sprite->Serialize(entityCtx.GetSubContext("Sprite"));
            }
            if (auto* camera = ecsRegistry.try_get<Camera>(entity))
            {
                camera->Serialize(entityCtx.GetSubContext("Camera"));
            }
            if (auto* anim = ecsRegistry.try_get<Animator>(entity))
            {
                anim->Serialize(entityCtx.GetSubContext("Animator"));

                if (anim->GetSprite())
                {
                    entityCtx.Write("spriteEntity",
                        static_cast<uint32_t>(anim->GetSprite()->GetGameObject()->GetEntity()));
                }
                else
                {
                    entityCtx.Write("spriteEntity", static_cast<uint32_t>(entt::null));
                }
            }
            if (auto* pb = ecsRegistry.try_get<ProgressBar>(entity))
            {
                pb->Serialize(entityCtx.GetSubContext("ProgressBar"));

                entityCtx.Write("bgEntity", pb->GetBackgroundEntity()
                    ? static_cast<uint32_t>(pb->GetBackgroundEntity()->GetGameObject()->GetEntity())
                    : static_cast<uint32_t>(entt::null));

                entityCtx.Write("fgEntity", pb->GetForegroundEntity()
                    ? static_cast<uint32_t>(pb->GetForegroundEntity()->GetGameObject()->GetEntity())
                    : static_cast<uint32_t>(entt::null));

                entityCtx.Write("indicatorEntity", pb->GetIndicatorTransform()
                    ? static_cast<uint32_t>(pb->GetIndicatorTransform()->GetGameObject()->GetEntity())
                    : static_cast<uint32_t>(entt::null));
            }
            if (auto* follow = ecsRegistry.try_get<Follow>(entity))
            {
                follow->Serialize(entityCtx.GetSubContext("Follow"));

                entityCtx.Write("targetEntity", follow->GetTarget()
                    ? static_cast<uint32_t>(follow->GetTarget()->GetGameObject()->GetEntity())
                    : static_cast<uint32_t>(entt::null));
            }
            if (auto* AC = ecsRegistry.try_get<AnimatorController>(entity))
            {
                AC->Serialize(entityCtx.GetSubContext("AnimatorController"));

                entityCtx.Write("mainEntity", AC->GetMainAnimator()
                    ? static_cast<uint32_t>(AC->GetMainAnimator()->GetGameObject()->GetEntity())
                    : static_cast<uint32_t>(entt::null));

                entityCtx.Write("overlayEntity", AC->GetOverlayAnimator()
                    ? static_cast<uint32_t>(AC->GetOverlayAnimator()->GetGameObject()->GetEntity())
                    : static_cast<uint32_t>(entt::null));
            }
            if (auto* audSrc = ecsRegistry.try_get<AudioSource>(entity))
            {
                audSrc->Serialize(entityCtx.GetSubContext("AudioSource"));
            }
            if (auto* audLis = ecsRegistry.try_get<AudioListener>(entity))
            {
                audLis->Serialize(entityCtx.GetSubContext("AudioListener"));
            }
            if (auto* rigid = ecsRegistry.try_get<Rigidbody>(entity))
            {
                rigid->Serialize(entityCtx.GetSubContext("Rigidbody"));
            }
            if (auto* coll = ecsRegistry.try_get<Collider>(entity))
            {
                coll->Serialize(entityCtx.GetSubContext("Collider"));
            }
            if (auto* text = ecsRegistry.try_get<Text>(entity))
            {
                text->Serialize(entityCtx.GetSubContext("Text"));
            }
            if (auto* netId = ecsRegistry.try_get<NetID>(entity))
            {
                netId->Serialize(entityCtx.GetSubContext("NetID"));
			}
            if(auto* syncTrans = ecsRegistry.try_get<SyncTransform>(entity))
            {
                syncTrans->Serialize(entityCtx.GetSubContext("SyncTransform"));
			}
            if (auto* tileMap = ecsRegistry.try_get<TileMap>(entity))
            {
                tileMap->Serialize(entityCtx.GetSubContext("TileMap"));
            }

            if (auto* parent = ecsRegistry.try_get<Parent>(entity))
            {
                entityCtx.Write("parent", static_cast<uint32_t>(parent->GetId()));
            }
            else
            {
                entityCtx.Write("parent", static_cast<uint32_t>(entt::null));
            }

            GameObject* obj = Game::GetGameObject(entity);

            if (!obj->GetScriptIDSet().empty())
            {
                entityCtx.BeginArray("Script");
                for (auto id : obj->GetScriptIDSet())
                {
                    entityCtx.WriteArrayItem(static_cast<uint32_t>(id));
                }
                entityCtx.EndArray();
            }

            entityCtx.Write("layer", static_cast<uint32_t>(obj->GetLayerMask().GetEnum()));
        }

        ctx.EndArray();
    }

    void Scene::Deserialize(const Online::Serialize::DeserializeContext& ctx)
    {
        for (auto& pair : entityToGameObject)
            gameObjectPool.Release(pair.second);
        entityToGameObject.clear();
        ecsRegistry.clear();
        delayDestroyQueue.clear();
        root = entt::null;
        CameraInited = false;

        int sceneVersion = 0;
        if (!ctx.Read("version", sceneVersion))
            Online::Log::Warning("Scene deserialize: No version found, assuming version 0");
        if (sceneVersion > 1)
        {
            Online::Log::Error("Scene deserialize: Unsupported version " + std::to_string(sceneVersion));
            return;
        }

        const Serialize::DeserializeContext& entitiesCtx = ctx.GetSubContext("entities");
        size_t entityCount = 0;
        if (!entitiesCtx.GetArraySize("", entityCount))
        {
            Online::Log::Warning("Scene deserialize: No entities found in file");
            return;
        }

        std::unordered_map<uint32_t, entt::entity> idMap;
        idMap.reserve(entityCount);
        for (size_t i = 0; i < entityCount; ++i)
        {
            const Serialize::DeserializeContext& entityCtx = entitiesCtx.GetArrayElement(i);
            uint32_t oldId = 0;
            if (!entityCtx.Read("id", oldId))
            {
                Online::Log::Error("Scene deserialize: Entity " + std::to_string(i) + " has no id, skipping");
                continue;
            }
            entt::entity newEntity = ecsRegistry.create();
            idMap[oldId] = newEntity;
            ecsRegistry.emplace<Tag>(newEntity, "unnamed", "default");
            ecsRegistry.emplace<Transform>(newEntity).Reset();
            ecsRegistry.emplace<ChildLink>(newEntity);
        }
        if (idMap.empty())
        {
            Online::Log::Warning("Scene deserialize: No valid entities created");
            return;
        }

        std::unordered_map<entt::entity, Online::Render::RenderLayer> entityLayerMap;
        std::unordered_map<uint32_t, EntityRefFixup> refFixups;
        refFixups.reserve(entityCount);

        for (size_t i = 0; i < entityCount; ++i)
        {
            const Serialize::DeserializeContext& entityCtx = entitiesCtx.GetArrayElement(i);
            uint32_t oldId = 0;
            if (!entityCtx.Read("id", oldId) || !idMap.count(oldId))
                continue;

            entt::entity entity = idMap[oldId];
            EntityRefFixup fixup;

            if (entityCtx.HasSubContext("Tag"))
            {
                auto& tag = ecsRegistry.get<Tag>(entity);
                tag.Deserialize(entityCtx.GetSubContext("Tag"));
            }
            if (entityCtx.HasSubContext("Transform"))
            {
                auto& trans = ecsRegistry.get<Transform>(entity);
                trans.Deserialize(entityCtx.GetSubContext("Transform"));
            }
            if (entityCtx.HasSubContext("Sprite"))
            {
                auto& sprite = ecsRegistry.emplace<Sprite>(entity);
                sprite.Deserialize(entityCtx.GetSubContext("Sprite"));
            }
            if (entityCtx.HasSubContext("Camera"))
            {
                auto& cam = ecsRegistry.emplace<Camera>(entity);
                cam.Deserialize(entityCtx.GetSubContext("Camera"));
            }
            if (entityCtx.HasSubContext("Animator"))
            {
                auto& anim = ecsRegistry.emplace<Animator>(entity);
                anim.Deserialize(entityCtx.GetSubContext("Animator"));
                fixup.hasAnimator = true;
                if (!entityCtx.Read("spriteEntity", fixup.spriteEntityId))
                    fixup.spriteEntityId = static_cast<uint32_t>(entt::null);
            }
            if (entityCtx.HasSubContext("ProgressBar"))
            {
                auto& pb = ecsRegistry.emplace<ProgressBar>(entity);
                pb.Deserialize(entityCtx.GetSubContext("ProgressBar"));
                fixup.hasProgressBar = true;
                entityCtx.Read("bgEntity", fixup.bgEntityId);
                entityCtx.Read("fgEntity", fixup.fgEntityId);
                entityCtx.Read("indicatorEntity", fixup.indicatorEntityId);
            }
            if (entityCtx.HasSubContext("Follow"))
            {
                auto& follow = ecsRegistry.emplace<Follow>(entity);
                follow.Deserialize(entityCtx.GetSubContext("Follow"));
                fixup.hasFollow = true;
                entityCtx.Read("targetEntity", fixup.targetEntityId);
            }
            if (entityCtx.HasSubContext("AnimatorController"))
            {
                auto& AC = ecsRegistry.emplace<AnimatorController>(entity);
                AC.Deserialize(entityCtx.GetSubContext("AnimatorController"));
                fixup.hasAnimatorController = true;
                entityCtx.Read("mainEntity", fixup.mainEntityId);
                entityCtx.Read("overlayEntity", fixup.overlayEntityId);
            }
            if (entityCtx.HasSubContext("AudioSource"))
            {
                auto& src = ecsRegistry.emplace<AudioSource>(entity);
                src.Deserialize(entityCtx.GetSubContext("AudioSource"));
            }
            if (entityCtx.HasSubContext("AudioListener"))
            {
                auto& lis = ecsRegistry.emplace<AudioListener>(entity);
                lis.Deserialize(entityCtx.GetSubContext("AudioListener"));
            }
            if (entityCtx.HasSubContext("Rigidbody"))
            {
                auto& rigi = ecsRegistry.emplace<Rigidbody>(entity);
                rigi.Deserialize(entityCtx.GetSubContext("Rigidbody"));
            }
            if (entityCtx.HasSubContext("Collider"))
            {
                auto& coll = ecsRegistry.emplace<Collider>(entity);
                coll.Deserialize(entityCtx.GetSubContext("Collider"));
            }
            if (entityCtx.HasSubContext("Text"))
            {
                auto& text = ecsRegistry.emplace<Text>(entity);
                text.Deserialize(entityCtx.GetSubContext("Text"));
            }
            if (entityCtx.HasSubContext("NetID"))
            {
                auto& netId = ecsRegistry.emplace<NetID>(entity);
                netId.Deserialize(entityCtx.GetSubContext("NetID"));
            }
            if (entityCtx.HasSubContext("SyncTransform"))
            {
                auto& syncTrans = ecsRegistry.emplace<SyncTransform>(entity);
                syncTrans.Deserialize(entityCtx.GetSubContext("SyncTransform"));
			}
            if (entityCtx.HasSubContext("TileMap"))
            {
                auto& tileMap = ecsRegistry.emplace<TileMap>(entity);
                tileMap.Deserialize(entityCtx.GetSubContext("TileMap"));
            }

            uint32_t layerRaw = 0;
            if (entityCtx.Read("layer", layerRaw))
                entityLayerMap[entity] = static_cast<Online::Render::RenderLayer>(layerRaw);

            refFixups[oldId] = fixup;
        }

        for (size_t i = 0; i < entityCount; ++i)
        {
            const Serialize::DeserializeContext& entityCtx = entitiesCtx.GetArrayElement(i);
            uint32_t oldId = 0;
            if (!entityCtx.Read("id", oldId) || !idMap.count(oldId))
                continue;
            entt::entity child = idMap[oldId];
            uint32_t parentId = static_cast<uint32_t>(entt::null);
            if (entityCtx.Read("parent", parentId) &&
                parentId != static_cast<uint32_t>(entt::null) &&
                idMap.count(parentId))
            {
                entt::entity parent = idMap[parentId];
                SetRelationship(child, parent, entt::null, false);
            }
        }

        uint32_t rootId = static_cast<uint32_t>(entt::null);
        if (ctx.Read("root", rootId) && idMap.count(rootId))
            root = idMap[rootId];
        else
        {
            root = entt::null;
            Online::Log::Warning("Scene deserialize: Root entity not found or invalid");
        }

        for (auto& [oldId, entity] : idMap)
        {
            GameObject* obj = gameObjectPool.Get();
            obj->Reset(entity);
            entityToGameObject[entity] = obj;

            if (auto* tag = ecsRegistry.try_get<Tag>(entity))
                tag->gameObject = obj;
            if (auto* trans = ecsRegistry.try_get<Transform>(entity))
                trans->gameObject = obj;
            if (auto* child = ecsRegistry.try_get<ChildLink>(entity))
                child->gameObject = obj;
            if (auto* sprite = ecsRegistry.try_get<Sprite>(entity))
                sprite->gameObject = obj;
            if (auto* cam = ecsRegistry.try_get<Camera>(entity))
                cam->gameObject = obj;
            if (auto* anim = ecsRegistry.try_get<Animator>(entity))
                anim->gameObject = obj;
            if (auto* pb = ecsRegistry.try_get<ProgressBar>(entity))
                pb->gameObject = obj;
            if (auto* follow = ecsRegistry.try_get<Follow>(entity))
                follow->gameObject = obj;
            if (auto* src = ecsRegistry.try_get<AudioSource>(entity))
                src->gameObject = obj;
            if (auto* lis = ecsRegistry.try_get<AudioListener>(entity))
                lis->gameObject = obj;
            if (auto* rigi = ecsRegistry.try_get<Rigidbody>(entity))
                rigi->gameObject = obj;
            if (auto* coll = ecsRegistry.try_get<Collider>(entity))
                coll->gameObject = obj;
            if (auto* text = ecsRegistry.try_get<Text>(entity))
                text->gameObject = obj;
            if (auto* netId = ecsRegistry.try_get<NetID>(entity))
				netId->gameObject = obj;
            if (auto* syncTrans = ecsRegistry.try_get<SyncTransform>(entity))
				syncTrans->gameObject = obj;
            if (auto* tileMap = ecsRegistry.try_get<TileMap>(entity))
                tileMap->gameObject = obj;
            if (auto* parent = ecsRegistry.try_get<Parent>(entity))
                parent->gameObject = obj;

            auto layerIt = entityLayerMap.find(entity);
            if (layerIt != entityLayerMap.end())
                obj->SetLayer(static_cast<Render::RenderLayer>(layerIt->second));

        }

        for (auto& [oldId, fixup] : refFixups)
        {
            if (!idMap.count(oldId)) continue;
            entt::entity entity = idMap[oldId];

            if (fixup.hasProgressBar)
            {
                auto* pb = ecsRegistry.try_get<ProgressBar>(entity);
                if (!pb) continue;
                if (fixup.bgEntityId != static_cast<uint32_t>(entt::null) && idMap.count(fixup.bgEntityId))
                    if (auto* bgSprite = ecsRegistry.try_get<Sprite>(idMap[fixup.bgEntityId]))
                        pb->SetBackgroundSprite(bgSprite);
                if (fixup.fgEntityId != static_cast<uint32_t>(entt::null) && idMap.count(fixup.fgEntityId))
                    if (auto* fgSprite = ecsRegistry.try_get<Sprite>(idMap[fixup.fgEntityId]))
                        pb->SetForegroundSprite(fgSprite);
                if (fixup.indicatorEntityId != static_cast<uint32_t>(entt::null) && idMap.count(fixup.indicatorEntityId))
                    if (auto* indTransform = ecsRegistry.try_get<Transform>(idMap[fixup.indicatorEntityId]))
                        pb->SetIndicatorTransform(indTransform);
            }
            if (fixup.hasFollow)
            {
                auto* follow = ecsRegistry.try_get<Follow>(entity);
                if (!follow) continue;
                if (fixup.targetEntityId != static_cast<uint32_t>(entt::null) && idMap.count(fixup.targetEntityId))
                    if (auto* targetTransform = ecsRegistry.try_get<Transform>(idMap[fixup.targetEntityId]))
                        follow->SetTarget(targetTransform);
            }
            if (fixup.hasAnimator)
            {
                auto* anim = ecsRegistry.try_get<Animator>(entity);
                if (!anim) continue;
                if (fixup.spriteEntityId != static_cast<uint32_t>(entt::null) && idMap.count(fixup.spriteEntityId))
                    if (auto* sprite = ecsRegistry.try_get<Sprite>(idMap[fixup.spriteEntityId]))
                        anim->SetSprite(sprite);
            }
            if (fixup.hasAnimatorController)
            {
                auto* AC = ecsRegistry.try_get<AnimatorController>(entity);
                if (!AC) continue;
                if (fixup.mainEntityId != static_cast<uint32_t>(entt::null) && idMap.count(fixup.mainEntityId))
                    if (auto* anim = ecsRegistry.try_get<Animator>(idMap[fixup.mainEntityId]))
                        AC->SetMainAnimator(anim);
                if (fixup.overlayEntityId != static_cast<uint32_t>(entt::null) && idMap.count(fixup.overlayEntityId))
                {
                    entt::entity overlayEnt = idMap[fixup.overlayEntityId];
                    if (auto* overlayAnim = ecsRegistry.try_get<Animator>(overlayEnt))
                        AC->SetOverlayAnimator(overlayAnim);
                }
                if (!AC->GetCurrentStateName().empty())
                    AC->Play(AC->GetCurrentStateName());
                else if (!AC->GetDefaultStateName().empty())
                    AC->Play(AC->GetDefaultStateName());
            }
        }

        for (size_t i = 0; i < entityCount; ++i)
        {
            const Serialize::DeserializeContext& entityCtx = entitiesCtx.GetArrayElement(i);
            uint32_t oldId = 0;
            if (!entityCtx.Read("id", oldId) || !idMap.count(oldId))
                continue;
            entt::entity entity = idMap[oldId];
            auto it = entityToGameObject.find(entity);
            if (it == entityToGameObject.end())
                continue;
            GameObject* obj = it->second;
            entityCtx.GetAllSubKeys();

            if (entityCtx.HasSubContext("Script"))
            {
                const auto& scriptCtx = entityCtx.GetSubContext("Script");
                size_t scriptCount = 0;
                if (scriptCtx.GetArraySize("", scriptCount))
                {
                    for (size_t j = 0; j < scriptCount; ++j)
                    {
                        uint32_t rawId = 0;
                        if (scriptCtx.GetArrayElement(j).Read("", rawId))
                            obj->AddScriptFunction(static_cast<Script::ScriptFunctionID>(rawId));
                    }
                }
            }
        }

        Online::Log::Info("Scene deserialize completed: " + std::to_string(idMap.size()) + " entities loaded");
    }

    void Scene::InitMainCamera()
    {
        entt::entity camEntity = entt::null;
        auto tagView = ecsRegistry.view<Tag>();
        for (auto [entity, tag] : tagView.each())
        {
            if (tag.GetName() == "Main Camera")
            {
                camEntity = entity;
                break;
            }
        }

        if (camEntity == entt::null)
        {
            camEntity = CreateEntity("Main Camera", "MainCamera");
            if (camEntity == entt::null)
            {
                return;
            }
        }

        Camera& camera = ecsRegistry.emplace_or_replace<Camera>(camEntity);
        camera.SetRenderTarget(Online::Asset::TextureID::Tex_WindowBuffer);
        camera.SetRenderSize({ 1280, 780 });

        Transform* trans = ecsRegistry.try_get<Transform>(camEntity);
        if (trans)
        {
            trans->SetLocalPosition({ 0.0f, 0.0f });
        }

        GameObject* camGo = GetGameObject(camEntity);
        if (!camGo)
        {
            camGo = gameObjectPool.Get();
            camGo->Reset(camEntity);
            entityToGameObject[camEntity] = camGo;

            if (auto* tg = ecsRegistry.try_get<Tag>(camEntity))
                tg->gameObject = camGo;
            if (auto* tr = ecsRegistry.try_get<Transform>(camEntity))
                tr->gameObject = camGo;
            if (auto* cl = ecsRegistry.try_get<ChildLink>(camEntity))
                cl->gameObject = camGo;
            if (auto* cm = ecsRegistry.try_get<Camera>(camEntity))
                cm->gameObject = camGo;
        }

        if (camGo->IsActive())
        {
            camera.OnEnable();
        }

        CameraInited = true;

        GameObject* target = Game::GetLocalPlayer();
        
        if (!target)
            return;
        Follow& follow = camGo->AddComponent<Follow>();
        follow.SetTarget(target->GetComponent<Transform>());
        follow.SetOffest({ -640,-390 });
        std::string name = std::string(target->GetName());
        IsEnableSync = true;

    }

    entt::entity Scene::GetRootEntity() const noexcept
    {
        return root;
    }

    GameObject* Scene::GetRootGameObject() const noexcept
    {
        auto it = entityToGameObject.find(root);
        return it != entityToGameObject.end() ? it->second : nullptr;
    }

    void Scene::CollectSyncEntities(std::vector<Net::EntityStateData>& outStates)
    {
        outStates.clear();
        auto syncView = ecsRegistry.view<NetID, Transform, Rigidbody>();
        outStates.reserve(syncView.size_hint());

        uint32_t frame = Game::GetServerFrame();    

        for (auto [entity, netId, trans, rb] : syncView.each())
        {
            if (!netId.GetNeedSync()) continue;

            GameObject* obj = GetGameObject(entity);
            if (obj && !obj->IsActive()) continue;

            Net::EntityStateData state{};
            state.netId = netId.GetNetId();
            state.x = trans.GetWorldPosition().x;
            state.y = trans.GetWorldPosition().y;
            state.rotation = trans.GetWorldRotation();

            auto vel = rb.GetVelocity(entity);
            state.velocityX = vel.x;
            state.velocityY = vel.y;

            state.serverFrame = frame;

            outStates.push_back(state);
        }
    }

    void Scene::BroadcastEntityStates()
    {
#ifdef PIXEL_SERVER
        std::vector<Net::EntityStateData> states;
        CollectSyncEntities(states);

        if (states.empty()) return;

        Net::EntityStatePacket pkt;
        pkt.entities = std::move(states);
        pkt.entityCount = static_cast<uint32_t>(pkt.entities.size());

        std::vector<std::byte> payload = pkt.SerializePayload();

        Net::Server::BroadcastUnreliable(payload,Net::PacketType::EntityState,Net::ChannelType::Unreliable);
#endif // PIXEL_SERVER
    }

    void Scene::ProcessEntityStatePacket(const Net::EntityStatePacket& pkt)
    {
#ifdef PIXEL_CLIENT
        if(IsEnableSync == false)
			return;
        std::unordered_map<uint32_t, const Net::EntityStateData*> stateMap;
        for (const auto& state : pkt.entities)
        {
            stateMap[state.netId] = &state;
        }

        float now = Time::seconds();

        auto view = ecsRegistry.view<NetID, SyncTransform, Transform>();
        for (auto [entity, netIdComp, syncComp, transform] : view.each())
        {
            uint32_t netId = netIdComp.GetNetId();
            auto it = stateMap.find(netId);
            if (it == stateMap.end())
                continue;

            const auto& state = *it->second;

            syncComp.targetX = state.x;
            syncComp.targetY = state.y;
            syncComp.targetRotation = state.rotation;
            syncComp.targetVelX = state.velocityX;
            syncComp.targetVelY = state.velocityY;
            syncComp.serverFrame = state.serverFrame;
            syncComp.receiveTime = now;
            syncComp.needInterpolation = true;

            stateMap.erase(it);
        }

        for (auto& [netId, state] : stateMap)
        {
            RequestEntityData(netId);
        }
#endif // PIXEL_CLIENT
    }

    void Scene::RequestEntityData(uint32_t netId)
    {
#ifdef PIXEL_CLIENT
        static std::unordered_set<uint32_t> pendingRequests;
        if (pendingRequests.count(netId)) return;
        pendingRequests.insert(netId);

        Net::ReqEntityDataPacket req;
        req.targetNetId = netId;
        auto payload = req.SerializePayload();
        Net::Client::SendReliable(payload, Net::PacketType::ReqEntityData, Net::ChannelType::ReliableOrdered);
#endif
    }

    void Scene::CreateEntityFromFullData(const Net::EntityFullData& data)
    {
        auto netView = ecsRegistry.view<NetID>();
        for (auto& entity : netView)
        {
            if (netView.get<NetID>(entity).GetNetId() == data.netId)
            {
                Online::Log::Warning("CreateEntityFromFullData: Entity with netId "
                    + std::to_string(data.netId) + " already exists, skipping.");
                return;
            }
        }

        // 2. 创建 GameObject 基础实体
        GameObject* go = CreateGameObject(data.entityName, data.entityTag);
        if (!go)
        {
            Online::Log::Error("CreateEntityFromFullData: Failed to create GameObject for netId "
                + std::to_string(data.netId));
            return;
        }

        // 3. 设置 Transform
        Transform* trans = go->GetTransform();
        if (trans)
        {
            trans->SetWorldPosition(data.position);
            trans->SetWorldScale(data.scale);
            trans->SetWorldRotation(data.rotation);
        }

        // 4. 添加并配置 NetID（客户端被动接收，不发起同步）
        NetID& netIdComp = go->AddComponent<NetID>();
        netIdComp.SetNetId(data.netId);
        netIdComp.SetOwnerConnId(data.ownerConnId);
        netIdComp.SetNeedSync(false);

        // 5. 添加 SyncTransform 组件，用于后续 EntityState 平滑插值
        go->AddComponent<SyncTransform>();

        // 6. 根据数据创建物理相关组件
        if (data.hasRigidbody)
        {
            Rigidbody& rb = go->AddComponent<Rigidbody>();
            rb.SetGravityScale(data.gravityScale);
            rb.SetFixedRotation(data.fixedRotation);
            rb.SetBodyType(data.bodyType);
        }

        if (data.hasCollider)
        {
            Collider& col = go->AddComponent<Collider>();
            col.SetShape(data.shape);
            col.SetHalfSize(data.halfSize);
            col.SetRadius(data.radius);
            col.SetDensity(data.density);
            col.SetFriction(data.friction);
            col.SetRestitution(data.restitution);
            Core::StateFlags<Physics::PhysicsLayer> categoryFlag;
            categoryFlag.OverwriteBits(static_cast<Physics::PhysicsLayer>(data.layerBits));
            col.SetCategory(categoryFlag);    
        }

        for (uint32_t scriptId : data.scriptIds)
        {
            go->AddScriptFunction(static_cast<Script::ScriptFunctionID>(scriptId));
        }

        if (data.hasCharacter)
        {
            Online::Game::AddAnimatorControll(data.roleId, go);
        }

        Online::Log::Info("CreateEntityFromFullData: Successfully created entity "
            + data.entityName + " (netId=" + std::to_string(data.netId) + ")");
    }

    void Scene::Update(float deltaTime)
    {
#ifdef PIXEL_CLIENT
        if (!CameraInited)
        {
            InitMainCamera();
        }
#endif

        for (auto& [entity, obj] : entityToGameObject) {
            if (!obj->IsActive()) continue;
            obj->ExecuteScriptUpdate(deltaTime);
        }
    }

    void Scene::LateUpdate(float deltaTime)
    {
        for (auto& [entity, obj] : entityToGameObject) {
            if (!obj->IsActive()) continue;
            obj->ExecuteScriptLateUpdate(deltaTime);
        }
    }

    entt::entity Scene::CreateEntity(std::string_view name, std::string_view tag)
    {
        entt::entity entity = ecsRegistry.create();

        ecsRegistry.emplace<Tag>(entity, name, tag);
        auto& transform = ecsRegistry.emplace<Transform>(entity);
        transform.Reset();
        ecsRegistry.emplace<ChildLink>(entity);

        if (root == entt::null)
        {
            root = entity;
        }
        else
        {
            entt::entity curr = root;
            while (ecsRegistry.get<ChildLink>(curr).GetNextSibling() != entt::null)
            {
                curr = ecsRegistry.get<ChildLink>(curr).GetNextSibling();
            }
            SetRelationship(entity, entt::null, curr);
        }
        return entity;
    }

    void Scene::ProcessTransformDirtySystem()
    {
        auto view = ecsRegistry.view<Transform>();
        for (auto [entity, transform] : view.each())
        {
            if (transform.IsLocalDirty())
            {
                MarkColliderTreeDirty(entity);
                transform.ClearLocalDirty();
            }
        }
    }

    void Scene::MarkColliderTreeDirty(entt::entity rootEntity)
    {
        if (auto* collider = ecsRegistry.try_get<Collider>(rootEntity))
            collider->SetRigidReady(false);

        if (auto* link = ecsRegistry.try_get<ChildLink>(rootEntity))
        {
            entt::entity child = link->GetFirstChild();
            while (child != entt::null)
            {
                MarkColliderTreeDirty(child);
                ChildLink* childLink = ecsRegistry.try_get<ChildLink>(child);
                child = childLink ? childLink->GetNextSibling() : entt::null;
            }
        }
    }

    GameObject* Scene::CreateGameObject(std::string_view name, std::string_view tag)
    {
        entt::entity entity = CreateEntity(name, tag);
        GameObject* ptr = gameObjectPool.Get();
        ptr->Reset(entity);
        entityToGameObject[entity] = ptr;

        if (auto* tagComp = ecsRegistry.try_get<Tag>(entity)) tagComp->gameObject = ptr;
        if (auto* transform = ecsRegistry.try_get<Transform>(entity)) transform->gameObject = ptr;
        if (auto* childLink = ecsRegistry.try_get<ChildLink>(entity)) childLink->gameObject = ptr;

#ifdef PIXEL_SERVER
        if (tag != "System" && tag != "Terrain")
        {
            NetID& netId = ecsRegistry.emplace<NetID>(entity);
            netId.SetNetId(Game::Generate());
            netId.SetNeedSync(true);
            ecsRegistry.emplace<SyncTransform>(entity);
        }
#endif

        return ptr;
    }

    void Scene::DestroyEntity(entt::entity entity)
    {
        if (!ecsRegistry.valid(entity)) { return; }
        if (GameObject* obj = GetGameObject(entity))
            obj->SetActive(false);
        if (auto* netId = ecsRegistry.try_get<NetID>(entity))
            netId->SetNeedSync(false);
        delayDestroyQueue.push_back(entity);
    }

    void Scene::SetRelationship(entt::entity childId, entt::entity parentId, entt::entity afterSibling, bool keepWorldTransform)
    {
        if (childId == parentId || !ecsRegistry.valid(childId)) { return; }
        if (parentId != entt::null && IsDescendant(parentId, childId)) { return; }

        auto* childTransform = ecsRegistry.try_get<Transform>(childId);
        auto* oldParentComp = ecsRegistry.try_get<Parent>(childId);
        entt::entity oldParentId = oldParentComp ? oldParentComp->GetId() : entt::null;

        if (oldParentId == parentId) { return; }

        glm::mat4 childWorldMatrix = glm::mat4(1.0f);
        if (childTransform && keepWorldTransform)
        {
            childWorldMatrix = childTransform->GetWorldMatrix();
        }

        UnlinkFromCurrent(childId);

        if (parentId != entt::null)
        {
            ecsRegistry.emplace_or_replace<Parent>(childId).SetId(parentId);
            if (!ecsRegistry.all_of<ChildLink>(parentId))
            {
                ecsRegistry.emplace<ChildLink>(parentId);
            }

            entt::entity insertAfter = afterSibling;
            if (insertAfter == entt::null)
            {
                ChildLink* parentLink = ecsRegistry.try_get<ChildLink>(parentId);
                if (parentLink)
                {
                    entt::entity curr = parentLink->GetFirstChild();
                    while (curr != entt::null)
                    {
                        ChildLink* currLink = ecsRegistry.try_get<ChildLink>(curr);
                        if (currLink && currLink->GetNextSibling() == entt::null)
                        {
                            insertAfter = curr;
                            break;
                        }
                        curr = currLink ? currLink->GetNextSibling() : entt::null;
                    }
                }
            }

            InsertIntoLinkList(childId, parentId, insertAfter);

            auto* newParentTransform = ecsRegistry.try_get<Transform>(parentId);
            if (newParentTransform && childTransform)
            {
                childTransform->SetParent(newParentTransform);
            }
        }
        else
        {
            ecsRegistry.remove<Parent>(childId);
            InsertIntoRootList(childId, afterSibling);

            if (childTransform)
            {
                childTransform->SetParent(nullptr);
            }
        }

        if (childTransform)
        {
            if (keepWorldTransform)
            {
                childTransform->SetWorldMatrix(childWorldMatrix);
            }
        }

        MarkColliderTreeDirty(childId);
        if (parentId != entt::null)
            MarkColliderTreeDirty(parentId);
    }

    GameObject* Scene::GetGameObject(entt::entity entity) noexcept
    {
        auto it = entityToGameObject.find(entity);
        return it != entityToGameObject.end() ? it->second : nullptr;
    }

    const GameObject* Scene::GetGameObject(entt::entity entity) const noexcept
    {
        auto it = entityToGameObject.find(entity);
        return it != entityToGameObject.end() ? it->second : nullptr;
    }

    void Scene::ProcessDelayDestroyQueue()
    {
        for (size_t i = 0; i < delayDestroyQueue.size(); ++i)
        {
            entt::entity entity = delayDestroyQueue[i];

            if (!ecsRegistry.valid(entity)) { continue; }

            auto it = entityToGameObject.find(entity);
            if (it != entityToGameObject.end())
            {
                gameObjectPool.Release(it->second);
                entityToGameObject.erase(it);
            }

            UnlinkFromCurrent(entity);

            auto* transform = ecsRegistry.try_get<Transform>(entity);
            if (transform)
            {
                std::vector<Transform*> childrenCopy = transform->GetChildren();
                for (Transform* child : childrenCopy)
                {
                    DestroyEntity(child->GetGameObject()->GetEntity());
                }
            }

            ecsRegistry.destroy(entity);
        }
        delayDestroyQueue.clear();
    }

    void Scene::ProcessAnimationSystem(float deltaTime)
    {
#ifdef PIXEL_CLIENT
        auto view = ecsRegistry.view<Animator>();
        for (auto [entity, animator] : view.each())
        {
            GameObject* obj = GetGameObject(entity);
            if (!obj || !obj->IsActive()) continue;

            if (!animator.GetSprite())
            {
                Log::Error("The object with the animation component does not have the sprite component");
                continue;
            }

            const Asset::AnimationClip* clip = animator.GetCurrentClip();
            if (!animator.IsPlaying() || animator.IsPaused() || !clip)
                continue;

            if (animator.GetNeedApplySettings())
            {
                animator.ApplyClipSettings(clip, animator.GetSprite());
                animator.SetNeedApplySettings(false);
            }

            animator.AddCurrentTime(deltaTime);
            float duration = clip->GetDuration();
            if (duration <= 0.0001f || clip->GetFrameCount() == 0)
                continue;

            if (clip->IsLooping())
            {
                while (animator.CurrentTime() >= duration)
                    animator.SubCurrentTime(duration); 
            }
            else
            {
                if (animator.CurrentTime() >= duration)
                {
                    animator.SetCurrentTime(duration);
                    animator.SetPlaying(false);
                }
            }

            uint8_t frameCount = clip->GetFrameCount();
            uint8_t newFrame = static_cast<uint8_t>(animator.CurrentTime() * clip->GetFrameRate());
            if (newFrame >= frameCount)
                newFrame = frameCount - 1;

            if (animator.GetCurrentFrameIndex() == 0xFF || newFrame != animator.GetCurrentFrameIndex())
            {
                animator.SetCurrentFrameIndex(newFrame);
                uint8_t targetFrame = clip->GetFrames()[animator.GetCurrentFrameIndex()];
                animator.GetSprite()->SetFrame(targetFrame);


                animator.ExecuteKeyframeEvents(newFrame);
            }
        }
#endif
    }

    void Scene::ProcessProgressBarSystem(float deltaTime)
    {
        auto view = ecsRegistry.view<ProgressBar>();
        for (auto [entity, pb] : view.each())
        {
            GameObject* obj = GetGameObject(entity);
            if (!obj || !obj->IsActive())
                continue;

            Sprite* bg = pb.GetBackgroundEntity();
            Sprite* fg = pb.GetForegroundEntity();

            if (!bg || !fg)
                continue;

            // 强制背景脱离进度模式
            if (bg->IsInProgressMode())
                bg->DisableProgressMode();

            // 强制前景置顶渲染顺序同步
            if (pb.IsForceForegroundOnTop() && pb.GetNeedSyncRenderOrder())
            {
                pb.SyncRenderOrder(bg, fg);
                pb.SetNeedSyncRenderOrder(false);
            }

            // 自动方向同步
            if (pb.IsAutoSyncDirection() && pb.GetNeedSyncDirection())
            {
                pb.SyncDirectionToSprite(fg);
                pb.SetNeedSyncDirection(false);
            }

            bool changed = false;
            float curr = pb.GetCurrentProgress();
            float targ = pb.GetTargetProgress();

            // 1. 基于平滑时间计算目标值（原始平滑算法）
            float next;
            if (!pb.IsSmoothPaused() && pb.GetSmoothTime() > 0.0001f)
            {
                float smoothTime = pb.GetSmoothTime();
                float factor = 1.0f - glm::exp(-deltaTime / smoothTime);
                next = glm::mix(curr, targ, factor);
            }
            else
            {
                next = targ;
            }

            // 2. 应用最大增长速度限制
            float maxDelta = pb.GetMaxSpeed() * deltaTime;
            if (maxDelta > 0.0f)
            {
                float delta = next - curr;
                if (delta > maxDelta)
                    next = curr + maxDelta;
                else if (delta < -maxDelta)
                    next = curr - maxDelta;
            }

            // 3. 应用变化到当前进度
            if (glm::abs(next - curr) > 0.0001f)
            {
                pb.SetCurrentProgress(next);
                changed = true;
            }

            // 4. 同步到前景精灵并触发回调
            if (changed)
            {
                fg->SetProgress(pb.GetCurrentProgress());

                auto& callback = pb.GetProgressChangeCallback();
                if (callback)
                    callback(pb.GetCurrentProgress(), pb.GetTargetProgress());

                pb.CheckAndTriggerComplete();
            }

            // 5. 处理指示器动画（原逻辑完整保留）
            if (pb.HasIndicatorAnimation())
            {
                Transform* indicatorTransform = pb.GetIndicatorTransform();
                Sprite* progressSprite = pb.GetForegroundEntity();
                if (indicatorTransform && progressSprite && progressSprite->GetGameObject())
                {
                    Transform* progressTransform = progressSprite->GetGameObject()->GetTransform();
                    if (progressTransform)
                    {
                        ProgressDirection dir = pb.GetDirection();
                        glm::vec2 worldPos = progressTransform->GetWorldPosition();
                        glm::vec2 renderOffset = progressSprite->GetRenderOffset();
                        float width = progressSprite->GetTextureSize().x;
                        float height = progressSprite->GetTextureSize().y;
                        float x, y;

                        switch (dir)
                        {
                        case ProgressDirection::LeftToRight:
                            x = worldPos.x + renderOffset.x + width * pb.GetCurrentProgress();
                            y = worldPos.y + renderOffset.y;
                            break;
                        case ProgressDirection::RightToLeft:
                            x = worldPos.x + renderOffset.x + width * (1.0f - pb.GetCurrentProgress());
                            y = worldPos.y + renderOffset.y;
                            break;
                        case ProgressDirection::TopToBottom:
                            x = worldPos.x + renderOffset.x;
                            y = worldPos.y + renderOffset.y + height * (1.0f - pb.GetCurrentProgress());
                            break;
                        case ProgressDirection::BottomToTop:
                            x = worldPos.x + renderOffset.x;
                            y = worldPos.y + renderOffset.y + height * pb.GetCurrentProgress();
                            break;
                        default:
                            break;
                        }
                        indicatorTransform->SetWorldPosition({ x, y });
                    }
                }
            }
        }
    }

    void Scene::ProcessFollowSystem(float deltaTime)
    {
        auto view = ecsRegistry.view<Follow, Transform>();

        for (auto [entity, follow, transform] : view.each())
        {

            GameObject* obj = GetGameObject(entity);
            if (!obj || !obj->IsActive())
                continue;

            if (!follow.GetEnableFollow() || !follow.GetTarget())
                continue;

            glm::vec2 targetPos = follow.GetTarget()->GetWorldPosition() + follow.GetOffest();
            glm::vec2 currentPos = transform.GetWorldPosition();

            glm::vec2 newPos{};

            if (follow.GetFollowMode() == FollowMode::Linear)
            {
                float t = (follow.GetLinearSpeed() * deltaTime > 1.0f) ? 1.0f : (follow.GetLinearSpeed() * deltaTime);
                newPos = glm::mix(currentPos, targetPos, t);
            }
            else if (follow.GetFollowMode() == FollowMode::Smooth)
            {
                float smoothFactor = 1.0f - glm::exp(-deltaTime / follow.GetSmoothTime());
                newPos = glm::mix(currentPos, targetPos, smoothFactor);
            }
            else
            {
                newPos = targetPos;
            }

            transform.SetWorldPosition(newPos);
        }
    }

    void Scene::ProcessColliderSystem()
    {
        for (auto [entity, collider] : ecsRegistry.view<Collider>().each())
        {
            if (collider.IsRigidReady())
                continue;

            entt::entity root = entity;
            glm::vec2 Offset{ 0,0 };
            float Angle = 0.0f;
            while (root != entt::null) 
            {
                if (HasComponent<Rigidbody>(root))
                {
                    collider.SetRigidEntity(root);
                    collider.SetExtraOffset(Offset);
                    collider.SetExtraAngle(Angle);
                    collider.SetRigidReady(true);
                    break;
                }
				Offset += GetComponent<Transform>(root)->GetLocalPosition();
                Angle += GetComponent<Transform>(root)->GetLocalRotation();
                root = GetComponent<Parent>(root) ? GetComponent<Parent>(root)->GetId() : entt::null;
            }
        }
    }

    void Scene::ProcessColliderListSystem()
    {
        for (auto [entity, colliderList] : ecsRegistry.view<ColliderList>().each())
        {
            for (auto& col : colliderList)
            {
                entt::entity root = entity;
                glm::vec2 Offset{ 0,0 };
                float Angle = 0.0f;
                while (root != entt::null)
                {
                    if (HasComponent<Rigidbody>(root))
                    {
                        col.SetRigidEntity(root);
                        col.SetExtraOffset(Offset);
                        col.SetExtraAngle(Angle);
                        break;
                    }
                    Offset += GetComponent<Transform>(root)->GetLocalPosition();
                    Angle += GetComponent<Transform>(root)->GetLocalRotation();
                    root = GetComponent<Parent>(root) ? GetComponent<Parent>(root)->GetId() : entt::null;
                }
            }
        }
    }

    void Scene::ProcessAnimatorControllerSystem(float deltaTime)
    {
#ifdef PIXEL_CLIENT
        auto view = ecsRegistry.view<AnimatorController>();
        for (auto [entity, controller] : view.each())
        {
            GameObject* obj = GetGameObject(entity);
            if (obj && !obj->IsActive()) 
                continue;

            Sprite* mainSpr = controller.GetMainSprite();
            if (!mainSpr) continue;

            for (auto& [name, param] : controller.GetParameters())
            {
                if (param.Type == AnimatorParameterType::Trigger && param.FloatValue != 0.0f)
                    param.FloatValue = 0.0f;
            }

            if (!controller.GetIsCrossfading())
            {
                for (const auto& trans : controller.GetTransitions())
                {
                    if (trans.SourceState != controller.GetCurrentStateName())
                        continue;

                    bool allMet = true;
                    for (const auto& cond : trans.Conditions)
                    {
                        const auto& params = controller.GetParameters();
                        auto pit = params.find(cond.ParameterName);
                        if (pit == params.end())
                        {
                            allMet = false;
                            break;
                        }
                        float val = pit->second.FloatValue;
                        bool met = false;
                        switch (cond.Mode)
                        {
                        case AnimatorConditionMode::If:      met = (val == cond.Threshold); break;
                        case AnimatorConditionMode::IfNot:   met = (val != cond.Threshold); break;
                        case AnimatorConditionMode::Greater: met = (val > cond.Threshold);  break;
                        case AnimatorConditionMode::Less:    met = (val < cond.Threshold);  break;
                        }
                        if (!met)
                        {
                            allMet = false;
                            break;
                        }
                    }

                    if (allMet)
                    {
                        if (trans.Duration <= 0.0f)
                            controller.Play(trans.DestState);
                        else
                            controller.Crossfade(trans.DestState, trans.Duration);
                        break;
                    }
                }
            }
            else
            {
                float newTimer = controller.GetCrossfadeTimer() + deltaTime;
                controller.SetCrossfadeTimer(newTimer);

                float t = newTimer / controller.GetCrossfadeDuration();
                if (t >= 1.0f)
                {
                    controller.EndCrossfade();
                }
                else
                {
                    mainSpr->SetAlpha(1.0f - t);
                    Sprite* overSpr = controller.GetOverlaySprite();
                    if (overSpr)
                        overSpr->SetAlpha(t);
                }
            }
        }
#endif
    }

    void Scene::ProcessSyncTransform()
    {
#ifdef PIXEL_CLIENT
        constexpr float INTERP_FACTOR = 0.2f;
        constexpr float MAX_PREDICTION_TIME = 0.1f;

        float now = Time::seconds();

        auto view = ecsRegistry.view<Transform, SyncTransform>();
        for (auto [entity, trans, sync] : view.each())
        {
            if (!sync.needInterpolation || sync.GetIsActive() == false)
                continue;

            float dt = now - sync.receiveTime;
            if (dt >= 6.0f)
            {
                Game::GetGameObject(entity)->SetActive(false);
            }
            dt = glm::clamp(dt, 0.0f, MAX_PREDICTION_TIME); 

            // 2. 外推位置 = 服务器位置 + 速度 * 时间差
            float predictedX = sync.targetX + sync.targetVelX * dt;
            float predictedY = sync.targetY + sync.targetVelY * dt;

            // 3. 平滑移动到预测位置
            glm::vec2 currentPos = trans.GetWorldPosition();
            float newX = currentPos.x + (predictedX - currentPos.x) * INTERP_FACTOR;
            float newY = currentPos.y + (predictedY - currentPos.y) * INTERP_FACTOR;

            // 旋转也做简单平滑
            float newRot = trans.GetWorldRotation() + (sync.targetRotation - trans.GetWorldRotation()) * INTERP_FACTOR;

            auto* rb = ecsRegistry.try_get<Rigidbody>(entity);
            if (rb)
            {
                // 直接设置刚体位置（物理世界立即更新）
                Physics::SetBodyTransform(entity, glm::vec2(newX, newY), newRot);
                // 同步服务器速度，让物理引擎从当前位置继续平滑模拟
                Physics::SetLinearVelocity(entity, glm::vec2(sync.targetVelX, sync.targetVelY));
                // 可选：设置角速度（若服务器同步了角速度，否则忽略）
                // float angVel = (newRot - trans.GetWorldRotation()) / Time::delta();
                // Physics::SetAngularVelocity(entity, angVel);
            }
            else
            {
                trans.SetWorldPosition({ newX, newY });
                trans.SetWorldRotation(newRot);
            }


            // 4. 如果已经很接近目标且无速度，停止插值
            constexpr float STOP_THRESHOLD = 0.5f;
            if (glm::abs(newX - predictedX) < STOP_THRESHOLD &&
                glm::abs(newY - predictedY) < STOP_THRESHOLD &&
                glm::abs(sync.targetVelX) < 0.01f && glm::abs(sync.targetVelY) < 0.01f)
            {
                sync.needInterpolation = false;
            }
        }
#endif
    }

    bool Scene::IsDescendant(entt::entity maybeChild, entt::entity maybeParent)
    {
        if (maybeChild == maybeParent) { return true; }
        if (!ecsRegistry.valid(maybeChild) || !ecsRegistry.valid(maybeParent)) { return false; }

        auto* childTrans = ecsRegistry.try_get<Transform>(maybeChild);
        auto* parentTrans = ecsRegistry.try_get<Transform>(maybeParent);
        if (!childTrans || !parentTrans) { return false; }

        const Transform* current = childTrans->GetParent();
        while (current)
        {
            if (current == parentTrans) { return true; }
            current = current->GetParent();
        }
        return false;
    }

    void Scene::UnlinkFromCurrent(entt::entity entity)
    {
        auto* link = ecsRegistry.try_get<ChildLink>(entity);
        if (!link) { return; }

        auto* transform = ecsRegistry.try_get<Transform>(entity);
        entt::entity parentId = entt::null;

        if (transform && transform->GetParent())
        {
            parentId = transform->GetParent()->GetGameObject()->GetEntity();
            transform->RemoveFromParent();
        }

        entt::entity prev = link->GetPreviousSibling();
        entt::entity next = link->GetNextSibling();

        if (prev != entt::null) { ecsRegistry.get<ChildLink>(prev).SetNextSibling(next); }
        else if (parentId != entt::null) { ecsRegistry.get<ChildLink>(parentId).SetFirstChild(next); }
        else if (entity == root) { root = next; }

        if (next != entt::null) { ecsRegistry.get<ChildLink>(next).SetPreviousSibling(prev); }

        link->SetNextSibling(entt::null);
        link->SetPreviousSibling(entt::null);
    }

    void Scene::InsertIntoLinkList(entt::entity child, entt::entity parent, entt::entity after)
    {
        if (!ecsRegistry.all_of<ChildLink>(child))
        {
            ecsRegistry.emplace<ChildLink>(child);
        }
        auto& pLink = ecsRegistry.get<ChildLink>(parent);
        auto& cLink = ecsRegistry.get<ChildLink>(child);

        if (after == entt::null)
        {
            entt::entity oldFirst = pLink.GetFirstChild();
            cLink.SetNextSibling(oldFirst);
            if (oldFirst != entt::null) { ecsRegistry.get<ChildLink>(oldFirst).SetPreviousSibling(child); }
            pLink.SetFirstChild(child);
        }
        else
        {
            auto& aLink = ecsRegistry.get<ChildLink>(after);
            entt::entity next = aLink.GetNextSibling();
            aLink.SetNextSibling(child);
            cLink.SetPreviousSibling(after);
            cLink.SetNextSibling(next);
            if (next != entt::null) { ecsRegistry.get<ChildLink>(next).SetPreviousSibling(child); }
        }
    }

    void Scene::InsertIntoRootList(entt::entity child, entt::entity after)
    {
        if (!ecsRegistry.all_of<ChildLink>(child))
        {
            ecsRegistry.emplace<ChildLink>(child);
        }
        auto& cLink = ecsRegistry.get<ChildLink>(child);

        if (after == entt::null || root == entt::null)
        {
            if (root == entt::null)
            {
                root = child;
            }
            else
            {
                entt::entity curr = root;
                while (curr != entt::null)
                {
                    ChildLink* currLink = ecsRegistry.try_get<ChildLink>(curr);
                    if (currLink && currLink->GetNextSibling() == entt::null)
                    {
                        currLink->SetNextSibling(child);
                        cLink.SetPreviousSibling(curr);
                        break;
                    }
                    curr = currLink ? currLink->GetNextSibling() : entt::null;
                }
            }
        }
        else
        {
            auto& aLink = ecsRegistry.get<ChildLink>(after);
            entt::entity next = aLink.GetNextSibling();
            aLink.SetNextSibling(child);
            cLink.SetPreviousSibling(after);
            cLink.SetNextSibling(next);
            if (next != entt::null) { ecsRegistry.get<ChildLink>(next).SetPreviousSibling(child); }
        }
    }

    void Scene::OnTriggerEnter(const Online::Event::Event& event)
    {
        auto& args = event.As<Online::Event::PhysTriggerEventArgs>();
        GameObject* self = GetGameObject(args.triggerEntity);
        GameObject* other = GetGameObject(args.otherEntity);
        if (!self || !other || !self->IsActive()) return;

        for (auto id : self->GetScriptIDSet()) 
        {
            auto* info = Script::GetInfo(id);
            if (info && info->onTriggerEnter)
                info->onTriggerEnter(self, other);
        }
    }

    void Scene::OnTriggerExit(const Online::Event::Event& event)
    {
        auto& args = event.As<Online::Event::PhysTriggerEventArgs>();
        GameObject* self = GetGameObject(args.triggerEntity);
        GameObject* other = GetGameObject(args.otherEntity);
        if (!self || !other || !self->IsActive()) return;

        for (auto id : self->GetScriptIDSet()) 
        {
            auto* info = Script::GetInfo(id);
            if (info && info->onTriggerExit)
                info->onTriggerExit(self, other);
        }
    }

    void Scene::OnTriggerStay(const Online::Event::Event& event)
    {
        auto& args = event.As<Online::Event::PhysTriggerEventArgs>();
        GameObject* self = GetGameObject(args.triggerEntity);
        GameObject* other = GetGameObject(args.otherEntity);
        if (!self || !other || !self->IsActive()) return;

        for (auto id : self->GetScriptIDSet())
        {
            auto* info = Script::GetInfo(id);
            if (info && info->onTriggerStay)
                info->onTriggerStay(self, other);
        }
    }

    void Scene::OnPhysFixedUpdate(const Online::Event::Event& event)
    {
        auto& args = event.As<Online::Event::PhysFixedUpdateEventArgs>();
        for (auto& [entity, obj] : entityToGameObject) 
        {
            if (!obj->IsActive()) continue;
            obj->ExecuteScriptFixedUpdate();
        }
    }

    void Scene::OnTriggerEnterThunk(void* listener, const Online::Event::Event& event)
    {
        static_cast<Scene*>(listener)->OnTriggerEnter(event);
    }

    void Scene::OnTriggerExitThunk(void* listener, const Online::Event::Event& event)
    {
        static_cast<Scene*>(listener)->OnTriggerExit(event);
    }

    void Scene::OnTriggerStayThunk(void* listener, const Online::Event::Event& event)
    {
        static_cast<Scene*>(listener)->OnTriggerStay(event);
    }

    void Scene::OnFixedUpdateThunk(void* listener, const Online::Event::Event& event)
    {
        static_cast<Scene*>(listener)->OnPhysFixedUpdate(event);
    }
}