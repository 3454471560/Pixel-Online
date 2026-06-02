#pragma once

#include<cstdint>

namespace Online::Physics
{
    enum class PhysicsLayer : uint16_t
    {
        Default = 1U << 0,
        Transparent = 1U << 1,
        UI = 1U << 2,
        Player = 1U << 3,
        Monster = 1U << 4,
        Terrain = 1U << 5
    };
}