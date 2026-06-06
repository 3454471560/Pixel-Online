#pragma once

#include <Context/Context.h>
#include <Client/Context/ClientContext.h>
#include <Asset/AssetHub.h>
#include <Asset/Common/ID/TextureID.h>
#include <Asset/Common/ID/SoundID.h>
#include <Asset/Common/ID/MusicID.h>
#include <Asset/Common/ID/FontID.h>
#include <Asset/Common/ID/AnimationClipID.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <glm.hpp>
#include <filesystem>

#include <stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Asset::AssetHub>
    {
        friend class Online::Runtime::Client;

    private:
        FuncTable() = default;
        ~FuncTable() = default;

    public:
        FuncTable(const FuncTable&) = delete;
        FuncTable& operator=(const FuncTable&) = delete;
        FuncTable(FuncTable&&) = delete;
        FuncTable& operator=(FuncTable&&) = delete;

    public:
        bool Check() const
        {
            if (!OnSetRenderer)        throw std::runtime_error("FuncTable miss [Asset::SetRenderer]!");
            if (!OnSyncLoadedAssets)   throw std::runtime_error("FuncTable miss [Asset::SyncLoadedAssets]!");
            if (!OnInitOffScreen)      throw std::runtime_error("FuncTable miss [Asset::OnInitOffScreen]!");
            if (!OnSaveTextureToPNG)   throw std::runtime_error("FuncTable miss [Asset::OnSaveTextureToPNG]!");
            if (!OnSaveScreenToPNG)    throw std::runtime_error("FuncTable miss [Asset::OnSaveScreenToPNG]!");
            if (!OnIsTextureReady)     throw std::runtime_error("FuncTable miss [Asset::OnIsTextureReady]!");
            if (!OnIsSoundReady)       throw std::runtime_error("FuncTable miss [Asset::OnIsSoundReady]!");
            if (!OnIsMusicReady)       throw std::runtime_error("FuncTable miss [Asset::OnIsMusicReady]!");
            if (!OnIsFontReady)        throw std::runtime_error("FuncTable miss [Asset::OnIsFontReady]!");
            if (!OnIsAnimReady)        throw std::runtime_error("FuncTable miss [Asset::OnIsAnimReady]!");
            if (!OnGetAssetLoadProgress) throw std::runtime_error("FuncTable miss [Asset::OnGetAssetLoadProgress]!");
            if (!GetTexture)           throw std::runtime_error("FuncTable miss [Asset::GetTexture]!");
            if (!GetSound)             throw std::runtime_error("FuncTable miss [Asset::GetSound]!");
            if (!GetMusic)             throw std::runtime_error("FuncTable miss [Asset::GetMusic]!");
            if (!GetFont)              throw std::runtime_error("FuncTable miss [Asset::GetFont]!");
            if (!GetAnim)              throw std::runtime_error("FuncTable miss [Asset::GetAnim]!");
            if (!GetTextureSize)       throw std::runtime_error("FuncTable miss [Asset::GetTextureSize]!");
            if (!GetFontSize)          throw std::runtime_error("FuncTable miss [Asset::GetFontSize]!");
            if (!OnGetFontAtlasCoord)  throw std::runtime_error("FuncTable miss [Asset::OnGetFontAtlasCoord]!");
            if (!OnGetFontAtlasSrcRect) throw std::runtime_error("FuncTable miss [Asset::OnGetFontAtlasSrcRect]!");
            if (!OnGetFontAtlasAdvance) throw std::runtime_error("FuncTable miss [Asset::OnGetFontAtlasAdvance]!");

            return true;
        }

        void UnRegister() noexcept
        {
            OnSetRenderer = nullptr;
            OnSyncLoadedAssets = nullptr;
            OnInitOffScreen = nullptr;
            OnSaveTextureToPNG = nullptr;
            OnSaveScreenToPNG = nullptr;
            OnIsTextureReady = nullptr;
            OnIsSoundReady = nullptr;
            OnIsMusicReady = nullptr;
            OnIsFontReady = nullptr;
            OnIsAnimReady = nullptr;
            OnGetAssetLoadProgress = nullptr;
            GetTexture = nullptr;
            GetSound = nullptr;
            GetMusic = nullptr;
            GetFont = nullptr;
            GetAnim = nullptr;
            GetTextureSize = nullptr;
            GetFontSize = nullptr;
            OnGetFontAtlasCoord = nullptr;
            OnGetFontAtlasSrcRect = nullptr;
            OnGetFontAtlasAdvance = nullptr;
        }

    public:
        void InvokeOnSetRenderer(SDL_Renderer* renderer) const
        {
            OnSetRenderer(renderer);
        }

        void InvokeOnSyncLoadedAssets() const
        {
            OnSyncLoadedAssets();
        }

        void InvokeOnInitOffScreen() const
        {
            OnInitOffScreen();
        }

        void InvokeOnSaveTextureToPNG(SDL_Texture* texture) const
        {
            OnSaveTextureToPNG(texture);
        }

        void InvokeOnSaveScreenToPNG(const std::filesystem::path& path) const
        {
            OnSaveScreenToPNG(path);
        }

        bool InvokeOnIsTextureReady(Online::Asset::TextureID id) const
        {
            return OnIsTextureReady(id);
        }

        bool InvokeOnIsSoundReady(Online::Asset::SoundID id) const
        {
            return OnIsSoundReady(id);
        }

        bool InvokeOnIsMusicReady(Online::Asset::MusicID id) const
        {
            return OnIsMusicReady(id);
        }

        bool InvokeOnIsFontReady(Online::Asset::FontID id) const
        {
            return OnIsFontReady(id);
        }

        bool InvokeOnIsAnimReady(Online::Asset::AnimationClipID id) const
        {
            return OnIsAnimReady(id);
        }

        float InvokeOnGetAssetLoadProgress() const
        {
            return OnGetAssetLoadProgress();
        }

        SDL_Texture* InvokeGetTexture(Online::Asset::TextureID id)
        {
            return GetTexture(id);
        }

        Mix_Chunk* InvokeGetSound(Online::Asset::SoundID id)
        {
            return GetSound(id);
        }

        Mix_Music* InvokeGetMusic(Online::Asset::MusicID id)
        {
            return GetMusic(id);
        }

        TTF_Font* InvokeGetFont(Online::Asset::FontID id)
        {
            return GetFont(id);
        }

        Online::Asset::AnimationClip* InvokeGetAnim(Online::Asset::AnimationClipID id)
        {
            return GetAnim(id);
        }

        glm::ivec2 InvokeGetTextureSize(Online::Asset::TextureID id)
        {
            return GetTextureSize(id);
        }

        int InvokeGetFontSize(Online::Asset::FontID id)
        {
            return GetFontSize(id);
        }

        glm::ivec2 InvokeOnGetFontAtlasCoord(char32_t codepoint) const
        {
            return OnGetFontAtlasCoord(codepoint);
        }

        SDL_Rect InvokeOnGetFontAtlasSrcRect(Online::Asset::FontID id, char32_t codepoint) const
        {
            return OnGetFontAtlasSrcRect(id, codepoint);
        }

        int InvokeOnGetFontAtlasAdvance(Online::Asset::FontID id, char32_t codepoint) const
        {
            return OnGetFontAtlasAdvance(id, codepoint);
        }

    private:
        void(*OnSetRenderer)(SDL_Renderer*) = nullptr;
        void(*OnSyncLoadedAssets)() = nullptr;
        void(*OnInitOffScreen)() = nullptr;
        void(*OnSaveTextureToPNG)(SDL_Texture*) = nullptr;
        void(*OnSaveScreenToPNG)(const std::filesystem::path&) = nullptr;
        bool(*OnIsTextureReady)(Online::Asset::TextureID) = nullptr;
        bool(*OnIsSoundReady)(Online::Asset::SoundID) = nullptr;
        bool(*OnIsMusicReady)(Online::Asset::MusicID) = nullptr;
        bool(*OnIsFontReady)(Online::Asset::FontID) = nullptr;
        bool(*OnIsAnimReady)(Online::Asset::AnimationClipID) = nullptr;
        float(*OnGetAssetLoadProgress)() = nullptr;
        SDL_Texture* (*GetTexture)(Online::Asset::TextureID) = nullptr;
        Mix_Chunk* (*GetSound)(Online::Asset::SoundID) = nullptr;
        Mix_Music* (*GetMusic)(Online::Asset::MusicID) = nullptr;
        TTF_Font* (*GetFont)(Online::Asset::FontID) = nullptr;
        Online::Asset::AnimationClip* (*GetAnim)(Online::Asset::AnimationClipID) = nullptr;
        glm::ivec2(*GetTextureSize)(Online::Asset::TextureID) = nullptr;
        int (*GetFontSize)(Online::Asset::FontID) = nullptr;
        glm::ivec2(*OnGetFontAtlasCoord)(char32_t) = nullptr;
        SDL_Rect(*OnGetFontAtlasSrcRect)(Online::Asset::FontID, char32_t) = nullptr;
        int (*OnGetFontAtlasAdvance)(Asset::FontID id, char32_t ch) = nullptr;
    };
}

namespace Online::Asset
{
    inline void SetRenderer(SDL_Renderer* renderer)
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnSetRenderer(renderer);
    }

    inline void SyncLoadedAssets()
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnSyncLoadedAssets();
    }

    inline void InitOffScreen()
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnInitOffScreen();
    }

    inline SDL_Texture* GetTexture(Online::Asset::TextureID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetTexture(id);
    }

    inline SDL_Texture* GetTexture(Online::Asset::FontID id)
    {
        Asset::TextureID ID;
        switch (id)
        {
        case Online::Asset::FontID::Ipix:
			ID = Online::Asset::TextureID::Tex_FontIpixAtlas;
            break;
        default:
            ID = Online::Asset::TextureID::Tex_FontIpixAtlas;
        }
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetTexture(ID);
    }

    inline Mix_Chunk* GetSound(Online::Asset::SoundID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetSound(id);
    }

    inline Mix_Music* GetMusic(Online::Asset::MusicID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetMusic(id);
    }

    inline TTF_Font* GetFont(Online::Asset::FontID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetFont(id);
    }

    inline Online::Asset::AnimationClip* GetAnim(Online::Asset::AnimationClipID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetAnim(id);
    }

    inline void SaveTextureToPNG(SDL_Texture* texture)
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnSaveTextureToPNG(texture);
    }

    inline void SaveScreenToPNG(const std::filesystem::path& path = "screenshot")
    {
        Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnSaveScreenToPNG(path);
    }

    inline bool IsTextureReady(Online::Asset::TextureID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnIsTextureReady(id);
    }

    inline bool IsAudioReady(Online::Asset::SoundID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnIsSoundReady(id);
    }

    inline bool IsMusicReady(Online::Asset::MusicID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnIsMusicReady(id);
    }

    inline bool IsFontReady(Online::Asset::FontID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnIsFontReady(id);
    }

    inline bool IsAnimReady(Online::Asset::AnimationClipID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnIsAnimReady(id);
    }

    inline glm::ivec2 GetTextureSize(Online::Asset::TextureID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetTextureSize(id);
    }

    inline float GetAssetLoadProgress()
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnGetAssetLoadProgress();
    }

    inline int GetFontSize(Online::Asset::FontID id)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeGetFontSize(id);
    }

    //inline glm::ivec2 GetFontAtlasCoord(char32_t codepoint)
    //{
    //    return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnGetFontAtlasCoord(codepoint);
    //}

    inline SDL_Rect GetFontAtlasSrcRect(Online::Asset::FontID id, char32_t codepoint)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnGetFontAtlasSrcRect(id, codepoint);
    }

    inline int GetFontAtlasAdvance(Online::Asset::FontID id, char32_t codepoint)
    {
        return Online::Runtime::ClientContext::Instance().GetClientFuncTable<AssetHub>().InvokeOnGetFontAtlasAdvance(id, codepoint);
    }
}