#pragma once
#include <Core/Color/Color.h>
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Game/Entity/GameObject.h>
#include <Input/Common/FuncTable.h>
namespace Online::Script
{
    struct Move
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::MoveLeftRight;
        class MoveData 
        {
            friend Move;
        private:
            float speed = 5.0f; float direction = 1.0f; 
        };
        static void MoveLeftRight_OnEnable(Game::GameObject* go)
        {
            auto* data = go->GetScriptData<MoveData>(ScriptFunctionID::MoveLeftRight);
            auto* rigi = go->GetComponent<Game::Rigidbody>();

            data->speed = 10.0f;
        }
        static void MoveLeftRight_Update(Game::GameObject* go, float dt)
        {
            auto* data = go->GetScriptData<MoveData>(ScriptFunctionID::MoveLeftRight);
            auto* trans = go->GetTransform();
			auto* rigi = go->GetComponent<Game::Rigidbody>();
            auto* anco = go->GetComponent<Game::AnimatorController>();
            auto* spri = go->GetComponent<Game::Sprite>();

            data->direction = 0.0f;

            if (Input::GetKeyDown(Input::KeyCode::A))
            {
                data->direction -= 1.0f;
                spri->SetFlipX(true);
            }

            if (Input::GetKeyDown(Input::KeyCode::D))
            {
                data->direction += 1.0f;
                spri->SetFlipX(false);
            }

            Physics::AddDebugRay(trans->GetWorldPosition(), { 0,1 }, 80, Core::Color::Red);

            if (data->direction != 0)
            {
                anco->SetFloat("Speed", 1.0f);
            }
            else
            {
                anco->SetFloat("Speed", 0.0f);
            }


            Physics::RayCastHit hit;

            Core::StateFlags<Physics::PhysicsLayer> layer;
            layer.SetBits(Physics::PhysicsLayer::Terrain);


            bool isLand = true;

            if (Input::GetKeyPressed(Input::KeyCode::Space))
            {
                if (Physics::RayCastLayer(trans->GetWorldPosition(), { 0,1 }, 80, hit, layer))
                {
                    rigi->AddImpulse(go->GetEntity(), { 0, -20 });
                }
            }

			rigi->SetVelocity(go->GetEntity(), { data->direction * data->speed, rigi->GetVelocity(go->GetEntity()).y});
        }
        static void MoveLeftRight_OnTriggerEnter(Game::GameObject* self, Game::GameObject* other)
        {
            Game::Rigidbody* rb = self->GetComponent<Game::Rigidbody>();
            Game::Transform* trans = self->GetComponent<Game::Transform>();
        }
        static void MoveLeftRight_OnTriggerExit(Game::GameObject* self, Game::GameObject* other)
        {
        }
        static void MoveLeftRight_OnTriggerUpdate(Game::GameObject* self, Game::GameObject* other)
        {

        }
        static void MoveData_Construct(void* p)
        {
            new (p) MoveData();
        }
        static void MoveData_Destruct(void* p)
        {
            static_cast<MoveData*>(p)->~MoveData();
        }
        static void MoveDate_FixedUpdate(Game::GameObject* go)
        {
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
                MoveDate_FixedUpdate,
                MoveLeftRight_OnTriggerEnter,
                MoveLeftRight_OnTriggerExit,
                MoveLeftRight_OnTriggerUpdate
            };
        }
    };
}
