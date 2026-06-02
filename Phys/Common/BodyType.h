 #pragma once
#include <cstdint>

namespace Online::Physics
{
    enum class BodyType : uint8_t 
    {
        Static = 0,
        Kinematic = 1,
        Dynamic = 2
    };
}
