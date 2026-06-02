#pragma once
#include <cstdint>

namespace Online::Core
{
    enum class Anchor : uint8_t
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center
    };
}