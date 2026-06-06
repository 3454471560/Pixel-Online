#pragma once
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Game/Entity/GameObject.h>
#include <Game/Component/Text.h>
#include <string>

namespace Online::Script
{
    struct LoadingScene
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::LoadingScene;

        class Data
        {
            friend LoadingScene;
        private:
            float timer = 0.0f;
            static constexpr float switchInterval = 0.5f;
        };

        static void LoadingScene_OnEnable(Game::GameObject* go)
        {
            auto* data = go->GetScriptData<Data>(ID);
            if (data) data->timer = 0.0f;
        }

        static void LoadingScene_Update(Game::GameObject* go, float dt)
        {
            auto* data = go->GetScriptData<Data>(ID);
            if (!data) return;

            auto* textComp = go->GetComponent<Game::Text>();
            if (!textComp) return;

            data->timer += dt;
            int dots = static_cast<int>(data->timer / data->switchInterval) % 4;
            std::string text = "load" + std::string(dots, '.');
            textComp->SetText(text);
        }

        static void LoadingSceneData_Construct(void* p)
        {
            new (p) Data();
        }
        static void LoadingSceneData_Destruct(void* p)
        {
            static_cast<Data*>(p)->~Data();
        }

        static ScriptFunctionInfo Information()
        {
            return
            {
                ID,
                sizeof(Data),
                LoadingSceneData_Construct,
                LoadingSceneData_Destruct,
                LoadingScene_OnEnable,
                nullptr,
                LoadingScene_Update
            };
        }
    };
}