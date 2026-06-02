#pragma once
#include <cstdint>

namespace Online::Asset
{
    enum class LoadRequestType : uint8_t
    {
        Texture,
        Sound,
        Music,
        Font
    };
}