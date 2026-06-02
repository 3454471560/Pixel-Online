#pragma once
#include <box2d/box2d.h>

namespace Online::Physics
{
    struct B2BodyIdEqual
    {
        bool operator()(b2BodyId a, b2BodyId b) const
        {
            return a.index1 == b.index1 && a.world0 == b.world0 && a.generation == b.generation;
        }
    };
}