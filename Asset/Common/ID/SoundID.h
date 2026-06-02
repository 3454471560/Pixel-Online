#pragma once
#include <cstdint>

namespace Online::Asset
{
    enum class SoundID : uint8_t
    {
        Invalid,
        Count
    };

    inline std::string ToString(SoundID id) noexcept
    {
        switch (id)
        {
        default: return "";
        }
    }
}
