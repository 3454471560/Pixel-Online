#pragma once
#include <Phys/Common/ColliderDesc.h>

namespace Online::Physics
{
    struct ColliderDescHash
    {
        size_t operator()(const ColliderDesc& desc) const
        {
            size_t h = 0;
            auto combine = [&h](auto val)
                {
                    h ^= std::hash<decltype(val)>()(val) + 0x9e3779b9 + (h << 6) + (h >> 2);
                };

            combine(static_cast<int>(desc.shapeType));
            combine(desc.offset.x); combine(desc.offset.y);
            combine(desc.radius);
            combine(desc.halfSize.x); combine(desc.halfSize.y);
            combine(desc.angle);
            combine(desc.density);
            combine(desc.friction);
            combine(desc.restitution);
            combine(desc.isSensor);
            combine(desc.categoryBits);
            combine(desc.maskBits);
            combine(desc.groupIndex);

            return h;
        }
    };
}