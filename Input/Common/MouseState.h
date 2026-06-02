#pragma once
#include <cstdint>

namespace Online::Input
{
    struct MouseSate
    {
        double  mouseX = 0.0;
        double  mouseY = 0.0;
        double  mouseScrollX = 0.0;
        double  mouseScrollY = 0.0;
    };
}