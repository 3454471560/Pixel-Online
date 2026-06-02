#pragma once
#include <cstdint>

namespace Online::Game
{
    enum class ProgressDirection : uint8_t
    {
        LeftToRight,
        RightToLeft,
        TopToBottom,
        BottomToTop
    };
}