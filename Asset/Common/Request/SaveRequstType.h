#pragma once
#pragma once
#include <cstdint>

namespace Online::Asset
{
    enum class SaveRequestType : uint8_t
    {
        PNG,
        JPG,
        BMP
    };
}