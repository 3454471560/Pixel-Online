#pragma once
#include <cstdint>

namespace Online::Script
{
    enum class ScriptFunctionID : uint32_t 
    {
        MoveLeftRight,
        RotateOverTime,
        BeginScene,
        LoadingScene,
        Button,
        FollowOverTime,
        TextInput,
        HealthBarController,
        Count
    };
}