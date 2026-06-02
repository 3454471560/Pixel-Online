#pragma once
#include <Script/Common/ScriptFunctionID.h>

namespace Online::Game { class GameObject; }

namespace Online::Script
{
    struct ScriptFunctionInfo {
        ScriptFunctionID id;
        size_t dataSize;
        void (*constructor)(void*);
        void (*destructor)(void*);

        void (*onEnable)(Game::GameObject*);
        void (*onDisable)(Game::GameObject*);
        void (*onUpdate)(Game::GameObject*, float);
        void (*onLateUpdate)(Game::GameObject*, float);
        void (*onFixedUpdate)(Game::GameObject*);
        void (*onTriggerEnter)(Game::GameObject* self, Game::GameObject* other);
        void (*onTriggerExit) (Game::GameObject* self, Game::GameObject* other);
        void (*onTriggerStay) (Game::GameObject* self, Game::GameObject* other);
    };
}
