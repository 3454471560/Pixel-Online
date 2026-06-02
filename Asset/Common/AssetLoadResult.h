#pragma once

#include <Asset/Common/ID/SoundID.h>
#include <Asset/Common/ID/MusicID.h>
#include <Asset/Common/ID/FontID.h>
#include <Asset/Common/ID/TextureID.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <variant>

namespace Online::Asset
{
    struct TextureLoadResult
    {
        TextureID id;
        SDL_Surface* texture = nullptr;
        int width = 0;
        int height = 0;
        bool success = false;
    };

    struct SoundLoadResult
    {
        SoundID id;
        Mix_Chunk* chunk = nullptr;
        bool success = false;
    };

    struct MusicLoadResult
    {
        MusicID id;
        Mix_Music* chunk = nullptr;
        bool success = false;
    };

    struct FontLoadResult
    {
        FontID id;
        TTF_Font* font = nullptr;
        bool success = false;
    };

    using AssetLoadResult = std::variant<
        TextureLoadResult,
        SoundLoadResult,
        FontLoadResult
    >;
}