#pragma once
#include <Core/Allocate/Allocate.h>
#include <Core/Thread/Thread.h>
#include <Core/ThreadSafe/ThreadSafeQueue.h>
#include <Context/Common/Module.h>
#include <Asset/Common/ID/AnimationClipID.h>
#include <Asset/Common/Data/AnimationClip.h>
#include <Asset/Common/Request/LoadRequest.h>
#include <Asset/Common/Request/SaveRequest.h>
#include <Thread/Common/FuncTable.h>
#include <Config/Common/Info/AnimationInfo.h>
#include <TileEdit/Common/TileMap.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <glm.hpp>
#include <array>
#include <atomic>
#include <thread>
#include <unordered_map>

namespace Online::Asset
{
    class AssetHub
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<AssetHub>;
        private:
            static AssetHub* Create() 
            {
                return ONLINE_NEW(AssetHub); 
            }
            static void Destroy(AssetHub* hub) 
            { 
                ONLINE_DELETE(hub); 
            }
        };
        struct Lifecycle
        {
            friend class Online::Runtime::Module<AssetHub>;
        private:
            static bool Initialize(AssetHub* hub, const std::vector<Online::Config::AnimationInfo>& animationInfo,
                const std::vector<Online::TileEdit::TileMap>& tileMapInfo)
            {
                return hub->Initialize(animationInfo, tileMapInfo); 
            }
            static void Release(AssetHub* hub) { hub->Release(); }
        };
    private:
        AssetHub() = default;
        ~AssetHub() = default;
    public:
        AssetHub(const AssetHub&) = delete;
        AssetHub& operator=(const AssetHub&) = delete;
        AssetHub(AssetHub&&) = delete;
        AssetHub& operator=(AssetHub&&) = delete;

    public:
        SDL_Texture* GetTexture(TextureID id) const;
        Mix_Chunk* GetSound(SoundID id) const;
        Mix_Music* GetMusic(MusicID id) const;
        TTF_Font* GetFont(FontID id) const;
        AnimationClip* GetAnim(AnimationClipID id) const;

        bool IsTextureReady(TextureID id) const;
        bool IsSoundReady(SoundID id) const;
        bool IsMusicReady(MusicID id) const;
        bool IsFontReady(FontID id) const;
        bool IsAnimReady(Online::Asset::AnimationClipID id) const;

        void LoadTextureAsync(TextureID id, const std::filesystem::path& path);
        void LoadSoundAsync(SoundID id, const std::filesystem::path& path);
        void LoadFontAsync(FontID id, const std::filesystem::path& path, int fontSize = 24);

        void SetRenderer(SDL_Renderer* renderer);
        void SyncLoadedAssets();

        bool CreateOrResizeRenderTarget(TextureID id, int width, int height);
        glm::ivec2 GetTextureSize(TextureID id) const;
        void InitOffScreen();

        bool SaveTextureToPNG(SDL_Texture* texture);
        bool SaveWindowScreenshot(const std::filesystem::path& path = "screenshot.png");
        float GetAsstetLoadProgress();
    private:
        bool Initialize(const std::vector<Online::Config::AnimationInfo>& animationInfo,
            const std::vector<Online::TileEdit::TileMap>& tileMapInfo);
        void Release();

        inline static void BootstrapLogThread(void* assethub, void*)
        {
            static_cast<Online::Asset::AssetHub*>(assethub)->LoaderThreadFunc();
        }
        void LoaderThreadFunc();
        void LoadHardAsset();

        bool IsAllAssetsLoaded();

        void LoadBuiltinAssets();
        void LoadAnimationClipAssets(const std::vector<Online::Config::AnimationInfo>& animationInfo);
        void LoadTileMapAssets(const std::vector<Online::TileEdit::TileMap>& tileMapInfo);

        TextureLoadResult LoadTextureInternal(TextureID id, const std::filesystem::path& path);
        SoundLoadResult LoadSoundInternal(SoundID id, const std::filesystem::path& path);
        SoundLoadResult LoadMusicInternal(MusicID id, const std::filesystem::path& path);
        FontLoadResult LoadFontInternal(FontID id, const std::filesystem::path& path, int fontSize);
        TextureLoadResult LoadTileMapTextureInternal(TextureID id, const Online::TileEdit::TileMap& tilemap);
        TextureLoadResult LoadSurfaceInternal(TextureID id, SDL_Surface* surface);

        void ClearAllResources();

    private:
        static const std::unordered_map<TextureID, std::filesystem::path> BuiltinTexturePaths;
        static const std::unordered_map<SoundID, std::filesystem::path> BuiltinSoundPaths;
        static const std::unordered_map<MusicID, std::filesystem::path> BuiltinMusicPaths;
        static const std::unordered_map<FontID, std::pair<std::filesystem::path, int>> BuiltinFontPaths;
        static const std::unordered_map<AnimationClipID, std::pair<std::filesystem::path, int>> AnimationClips;

    private:
        std::array<SDL_Texture*, static_cast<uint8_t>(TextureID::Count)>         textures{};
        std::array<Mix_Chunk*, static_cast<uint8_t>(SoundID::Count)>             sounds{};
        std::array<Mix_Music*, static_cast<uint8_t>(MusicID::Count)>             musics{};
        std::array<TTF_Font*, static_cast<uint8_t>(FontID::Count)>               fonts{};
        std::array<AnimationClip*, static_cast<uint8_t>(AnimationClipID::Count)> anims{};

        std::array<std::atomic<bool>, static_cast<uint8_t>(TextureID::Count)>       textureReady{};
        std::array<std::atomic<bool>, static_cast<uint8_t>(SoundID::Count)>         soundReady{};
        std::array<std::atomic<bool>, static_cast<uint8_t>(MusicID::Count)>         musicReady{};
        std::array<std::atomic<bool>, static_cast<uint8_t>(FontID::Count)>          fontReady{};
        std::array<std::atomic<bool>, static_cast<uint8_t>(AnimationClipID::Count)> animReady{};

        std::array<glm::ivec2, static_cast<uint8_t>(TextureID::Count)> textureSizes{};

        std::vector<AnimationClip> animsPool;

        Online::Core::ThreadSafeQueue<LoadRequest>     requestQueue;
        Online::Core::ThreadSafeQueue<AssetLoadResult> resultQueue;

        Online::Core::Thread::Identifier loaderThread;
        std::atomic<bool> isRunning = false;

        std::atomic<bool> isLoadedBuiltinAssets = false;
        std::atomic<bool> isLoadedAnimationsAssets = false;
        std::atomic<bool> isLoadedTileMapAssets = false;

        SDL_Renderer* sdlRenderer = nullptr;
    };
}