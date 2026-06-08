#pragma once
#include <cstdint>

namespace Online::Core
{
    enum class Anchor : uint8_t
    {
        TopLeft,
        TopCenter,
        TopRight,
        BottomLeft,
		BottomCenter,
        BottomRight,
        Center,
		CenterLeft,
		CenterRight
    };
}