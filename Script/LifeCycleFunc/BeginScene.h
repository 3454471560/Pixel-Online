#pragma once
#include <Core/Color/Color.h>
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Game/Entity/GameObject.h>
#include <Asset/Common/FuncTable.h>
namespace Online::Script
{
    struct BeginScene
    {
        static const Script::ScriptFunctionID ID = Script::ScriptFunctionID::BeginScene;
        class Begin
        {
            friend BeginScene;
        private:
        };
        static void BeginScene_OnEnable(Game::GameObject* go)
        {
            auto* data = go->GetScriptData<Begin>(ID);

        }
        static void BeginScene_Update(Game::GameObject* go, float dt)
        {
            auto* data = go->GetScriptData<Begin>(ID);
            auto* progress = go->GetComponent<Game::ProgressBar>();

            float pace = Asset::GetAssetLoadProgress();
            
            progress->SetProgress(pace);

            if (progress->IsComplete())
            {
                Game::SwitchSceneAsync("LoadingScene");
            }
        }

        static void BeginSceneData_Construct(void* p)
        {
            new (p) Begin();
        }
        static void BeginSceneData_Destruct(void* p)
        {
            static_cast<Begin*>(p)->~Begin();
        }
        static ScriptFunctionInfo Information()
        {
            return
            {
                ID,
                sizeof(Begin),
                BeginSceneData_Construct,
                BeginSceneData_Destruct,
                BeginScene_OnEnable,
                nullptr,
                BeginScene_Update
            };
        }
    };
}
