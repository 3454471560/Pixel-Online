#pragma once
#include <cstdint>
namespace Online::Game
{
    enum class SrcRectMode : uint8_t
    {
        Manual,
        AutoGrid,
        Progress
    };
}