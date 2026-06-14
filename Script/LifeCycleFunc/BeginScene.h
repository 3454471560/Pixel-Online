#pragma once
#include <Core/Color/Color.h>
#include <Script/Common/ScriptFunctionID.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <Game/Entity/GameObject.h>
#include <Asset/Common/FuncTable.h>
#include <Task/Common/FuncTable.h>

#include <Net/Client/Common/FuncTable.h>
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

            static bool netInit = false;
            static bool RequestInit = false;

            float pace = 0;

            float AssetPace = Asset::GetAssetLoadProgress();
			pace += AssetPace * 0.5;
            if (AssetPace >= 0.899f && netInit == false)
            {
				netInit = true;
                Online::Task::PostJob([]() {
                    if (Online::Net::Client::Connect("127.0.0.1", 7778)) {
                        Online::Log::Info("Connected to server!");
                    }
                    }, "AsyncConnect");
            }
            bool IsConnected = Net::Client::IsConnected();
			pace += IsConnected * 0.15;

            if (IsConnected && RequestInit == false)
            {
                RequestInit = true;
				Game::SendJoinWorldRequest("Player", Net::Client::GetLocalConnId());
            }

            if (Game::IsPendingSceneReady())
            {
                pace += 0.35f;
            }
            
            progress->SetProgress(pace);

            if (progress->IsComplete())
            {
                Game::DisplayPendingScene();
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
