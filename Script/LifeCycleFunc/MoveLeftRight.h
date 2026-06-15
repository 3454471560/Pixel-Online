#pragma once
#include <Core/Color/Color.h>
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Game/Entity/GameObject.h>
#include <Game/Common/FuncTable.h>  
#include <Input/Common/FuncTable.h>
#include <Net/Client/Common/FuncTable.h>

#include <Net/Common/PlayerInputPacket.h>

namespace Online::Script
{
    struct Move
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::MoveLeftRight;
        class MoveData
        {
            friend Move;
        private:
            float speed = 10.0f;
            float direction = 1.0f;
        };

        static void MoveLeftRight_OnEnable(Game::GameObject* go)
        {
            if (!go) return;

            auto* data = go->GetScriptData<MoveData>(ScriptFunctionID::MoveLeftRight);
            if (!data) return;

            data->speed = 10.0f;
        }
        static void MoveLeftRight_Update(Game::GameObject* go, float dt)
        {
            if (!go) return;

            auto* data = go->GetScriptData<MoveData>(ID);
            auto* trans = go->GetTransform();
            auto* rigi = go->GetComponent<Game::Rigidbody>();
            auto* anco = go->GetComponent<Game::AnimatorController>();
            auto* spri = go->GetComponent<Game::Sprite>();

            if (!data || !trans || !rigi) return;

            data->direction = 0.0f;

            // 只有客户端和本地玩家才读取硬件输入
#ifdef PIXEL_CLIENT
            constexpr float MOVE_THRESHOLD = 0.05f; // 移动阈值，可按需调整
            if (true)
            {
                auto* netIdComp = go->GetComponent<Game::NetID>();
                if (netIdComp && netIdComp->GetNetId() != Online::Game::GetLocalPlayerNetId())
                {
                    if (anco && spri)
                    {
                        float velX = rigi->GetVelocity(go->GetEntity()).x;
                        if (std::abs(velX) > MOVE_THRESHOLD)
                        {
                            anco->SetFloat("Speed", 1.0f);
                            spri->SetFlipX(velX < 0);
                        }
                        else
                        {
                            anco->SetFloat("Speed", 0.0f);
                        }
                    }
                    return;
                }
            }

            bool keyA_Hold = Input::GetKeyDown(Input::KeyCode::A);
            bool keyD_Hold = Input::GetKeyDown(Input::KeyCode::D);
            bool keySpace_Press = Input::GetKeyPressed(Input::KeyCode::Space);

            if (keyA_Hold)
            {
                data->direction -= 1.0f;
                if (spri) spri->SetFlipX(true);
            }
            if (keyD_Hold)
            {
                data->direction += 1.0f;
                if (spri) spri->SetFlipX(false);
            }

            if (anco)
            {
                anco->SetFloat("Speed", data->direction != 0 ? 1.0f : 0.0f);
            }

            rigi->SetVelocity(go->GetEntity(), {
                data->direction * data->speed,
                rigi->GetVelocity(go->GetEntity()).y
                });

            Physics::RayCastHit hit;
            Core::StateFlags<Physics::PhysicsLayer> layer;
            layer.SetBits(Physics::PhysicsLayer::Terrain);
            if (keySpace_Press)
            {
                if (Physics::RayCastLayer(trans->GetWorldPosition(), { 0,1 }, 80, hit, layer))
                {
                    rigi->AddImpulse(go->GetEntity(), { 0, -20 });
                }
            }

            if (true)
            {
                auto* netIdComp = go->GetComponent<Game::NetID>();
                if (!netIdComp) return;

                // 1. 构造玩家输入包（完全对齐你的 PlayerInputPacket 结构）
                Online::Net::PlayerInputPacket inputPkt;
                inputPkt.netId = netIdComp->GetNetId();
                inputPkt.connId = Online::Net::Client::GetLocalConnId(); // 获取本地客户端连接ID
                inputPkt.keyA_Hold = keyA_Hold;
                inputPkt.keyD_Hold = keyD_Hold;
                inputPkt.keySpace_Press = keySpace_Press;

                // 2. 序列化为字节流（使用你项目统一的 Payload 接口）
                std::vector<std::byte> payload = inputPkt.SerializePayload();

                // 3. 获取客户端网络模块，走【可靠有序通道】发送（适配你的 ChannelType）
                if (!payload.empty())
                {
                    Net::Client::SendReliable(payload, Online::Net::PacketType::PlayerInput);
                }
            }
#endif // PIXEL_CLIENT

#ifdef PIXEL_SERVER
            auto* netId = go->GetComponent<Game::NetID>();
            if (!netId) return;

            // 读取客户端上传的持续移动按键
            if (Online::Input::IsClientKeyHold(netId->GetOwnerConnId(), Input::KeyCode::A))
            {
                data->direction -= 1.0f;
            }
            if (Online::Input::IsClientKeyHold(netId->GetOwnerConnId(), Input::KeyCode::D))
            {
                data->direction += 1.0f;
            }

            rigi->SetVelocity(go->GetEntity(), {
               data->direction * data->speed,
               rigi->GetVelocity(go->GetEntity()).y
                });

            // 服务端权威跳跃逻辑
            if (Online::Input::ConsumeClientTrigger(netId->GetOwnerConnId(), Input::KeyCode::Space))
            {
                Physics::RayCastHit hit;
                Core::StateFlags<Physics::PhysicsLayer> layer;
                layer.SetBits(Physics::PhysicsLayer::Terrain);

                if (Physics::RayCastLayer(trans->GetWorldPosition(), { 0,1 }, 80, hit, layer))
                {
                    rigi->AddImpulse(go->GetEntity(), { 0, -20 });
                }
            }           
#endif // PIXEL_SERVER
        }

        static void MoveLeftRight_OnTriggerEnter(Game::GameObject* self, Game::GameObject* other)
        {
            if (!self || !other) return;

            // 触发器逻辑：服务端执行业务，客户端只做表现
#ifdef PIXEL_SERVER
// 服务端：处理扣血、拾取等业务逻辑
#endif

#ifdef PIXEL_CLIENT
// 客户端：播放音效、特效等表现
#endif
        }

        static void MoveData_Construct(void* p)
        {
            if (!p) return;
            new (p) MoveData();
        }

        static void MoveData_Destruct(void* p)
        {
            if (!p) return;
            static_cast<MoveData*>(p)->~MoveData();
        }

        static void MoveData_FixedUpdate(Game::GameObject* go)
        {
            if (!go) return;

            auto* data = go->GetScriptData<MoveData>(ID);
            auto* rigi = go->GetComponent<Game::Rigidbody>();
            if (!data || !rigi) return;
        }

        static ScriptFunctionInfo Information()
        {
            return
            {
                ID,
                sizeof(MoveData),
                MoveData_Construct,
                MoveData_Destruct,
                MoveLeftRight_OnEnable,
                nullptr,
                MoveLeftRight_Update,
                nullptr,
                MoveData_FixedUpdate,
                MoveLeftRight_OnTriggerEnter
            };
        }
    };
}