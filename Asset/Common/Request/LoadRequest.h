#pragma once
#include <Asset/Common/Request/LoadRequestType.h>
#include <Asset/Common/AssetLoadResult.h>

#include <filesystem>

namespace Online::Asset
{
    struct LoadRequest
    {
        LoadRequestType type;
        std::variant<TextureID, SoundID, MusicID, FontID> id;
        std::filesystem::path path;
        int fontSize = 24;
    };
}