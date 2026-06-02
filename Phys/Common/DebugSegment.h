#pragma once

#include <glm.hpp>

namespace Online::Physics
{
    struct DebugSegment
    {
        glm::vec2 p1;
        glm::vec2 p2;
        glm::vec4 color;
    };
}
