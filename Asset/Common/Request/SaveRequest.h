#pragma once
#include <Asset/Common/Request/SaveRequstType.h>

#include <SDL.h>
#include <SDL_image.h>

#include <filesystem>

namespace Online::Asset
{
    struct ImageSaveRequest
    {
        SaveRequestType type = SaveRequestType::PNG;
        SDL_Surface* surface = nullptr;
        std::filesystem::path path;
        int quality = 50;
    };
}