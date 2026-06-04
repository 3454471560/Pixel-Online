#pragma once
#include <cstdint>
namespace Online::Game
{
    enum class AnimatorConditionMode : uint8_t
    {
        If,
        IfNot,
        Greater,
        Less,
    };
}