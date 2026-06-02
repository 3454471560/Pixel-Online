#pragma once
#include <cstdint>

namespace Online::Asset
{
    enum class MusicID : uint8_t
    {
        Invalid,
#pragma region HardCode
        Mus_BackGround,
        Mus_Man,
#pragma endregion

        Count
    };

    inline std::string ToString(MusicID id) noexcept
    {
        switch (id)
        {
        case MusicID::Mus_BackGround: return "Mus_BackGround";
        default: return "";
        }
    }
}
