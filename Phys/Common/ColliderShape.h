#pragma once
#include <cstdint>

namespace Online::Physics
{
    enum class ColliderShape : uint8_t 
    {
        Circle,
        Box,
        Capsule,
        TileGrid
    };
}
