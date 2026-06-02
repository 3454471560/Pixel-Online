#pragma once

#include<cstdint>

namespace Online::Render
{
    enum class RenderLayer : uint32_t
    {
        Empty = 0U,

        Default = 1U << 0,
        Transparent = 1U << 1,
        UI = 1U << 2,

    };
}