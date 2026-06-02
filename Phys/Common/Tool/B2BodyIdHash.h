#pragma once
#include <box2d/box2d.h>

namespace Online::Physics
{
    struct B2BodyIdHash
    {
        size_t operator()(b2BodyId id) const
        {
            return (static_cast<size_t>(id.index1) << 32) ^
                (static_cast<size_t>(id.world0) << 16) ^
                static_cast<size_t>(id.generation);
        }
    };
}