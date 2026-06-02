#pragma once
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Game/Entity/GameObject.h>
#include <Input/Common/FuncTable.h>
namespace Online::Script
{
    struct Rotate
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::RotateOverTime;
        class RoatateData
        {
            friend Rotate;
        public:
            void SetDirection(bool IsClockwise)
            {
                this->IsClockwise = IsClockwise;
            }
            float GetSpeed()
            {
                return IsClockwise ? speed : -speed;
            }
        private:
            float speed = 5.0f; bool IsClockwise = true; float time = 0.0f;
        };
        static void RoatateData_OnEnable(Game::GameObject* go)
        {
            auto* data = go->GetScriptData<RoatateData>(ScriptFunctionID::RotateOverTime);
            data->speed = 13.0f;
        }
        static void Roatate_Update(Game::GameObject* go, float dt)
        {
            auto* data = go->GetScriptData<RoatateData>(ScriptFunctionID::RotateOverTime);
            auto* trans = go->GetTransform();

            trans->SetLocalRotation(trans->GetLocalRotation() + data->GetSpeed() * dt);
        }
        static void RoatateData_Construct(void* p)
        {
            new (p) RoatateData();
        }
        static void RoatateData_Destruct(void* p)
        {
            static_cast<RoatateData*>(p)->~RoatateData();
        }
        static ScriptFunctionInfo Information()
        {
            return
            {
                ID,
                sizeof(RoatateData),
                RoatateData_Construct,
                RoatateData_Destruct,
                RoatateData_OnEnable,
                nullptr,
                Roatate_Update,
                nullptr
            };
        }
    };

}
