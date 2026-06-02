#pragma once
#include<glm.hpp>

namespace Online::Physics
{
    struct DebugRay
    {
        glm::vec2 origin;
        glm::vec2 direction;
        float length;
        glm::vec4 color;
    };
}
