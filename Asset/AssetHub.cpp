#include <Core/Utils/File.h>
#include <Core/String/String.h>
#include <Asset/AssetHub.h>
#include <Asset/AssetResources.h>
#include <Log/Common/FuncTable.h>
#include <Task/Common/FuncTable.h>

#include <algorithm>
#include <stdexcept>

namespace Online::Asset
{

    const std::unordered_map<TextureID, std::filesystem::path> AssetHub::BuiltinTexturePaths =
    {
        { TextureID::Tex_Default,               "Resource/Default.png" },
        { TextureID::Tex_BackGround_Near,       "Resource/AutumnForest/Background/1.png" },
        { TextureID::Tex_BackGround_Mid,        "Resource/AutumnForest/Background/2.png" },
        { TextureID::Tex_BackGround_Far,        "Resource/AutumnForest/Background/3.png" },
        { TextureID::Tex_Tileset,               "Resource/AutumnForest/Tileset/Tileset.png" },
        { TextureID::Tex_SilverHat_Attack_1,    "Resource/Player/SilverHat/ATTACK_1.png" },
        { TextureID::Tex_SilverHat_Attack_2,    "Resource/Player/SilverHat/ATTACK_2.png" },
        { TextureID::Tex_SilverHat_Attack_3,    "Resource/Player/SilverHat/ATTACK_3.png" },
        { TextureID::Tex_SilverHat_Dash,        "Resource/Player/SilverHat/DASH.png" },
        { TextureID::Tex_SilverHat_Dash_Attack, "Resource/Player/SilverHat/DASH_ATTACK.png" },
        { TextureID::Tex_SilverHat_Dash_Attack_Prepare, "Resource/Player/SilverHat/DASH_ATTACK_PREPARE.png" },
        { TextureID::Tex_SilverHat_Death,       "Resource/Player/SilverHat/DEATH.png" },
        { TextureID::Tex_SilverHat_Fall,        "Resource/Player/SilverHat/FALL.png" },
        { TextureID::Tex_SilverHat_Hurt,        "Resource/Player/SilverHat/HURT.png" },
        { TextureID::Tex_SilverHat_Idle,        "Resource/Player/SilverHat/IDLE.png" },
        { TextureID::Tex_SilverHat_Jump,        "Resource/Player/SilverHat/JUMP.png" },
        { TextureID::Tex_SilverHat_Run,         "Resource/Player/SilverHat/RUN.png" },
        { TextureID::Tex_SilverHat_Strong_Attack, "Resource/Player/SilverHat/STRONG_ATTACK.png" },
        { TextureID::Tex_SilverHat_Strong_Attack_Prepare, "Resource/Player/SilverHat/STRONG_ATTACK_PREPARE.png" },
        { TextureID::Tex_SilverHat_Throw,       "Resource/Player/SilverHat/THROW.png" },
        { TextureID::Tex_SilverHat_Blade_Effect,"Resource/Effect/merged_image.png" }
    };

    const std::unordered_map<SoundID, std::filesystem::path> AssetHub::BuiltinSoundPaths =
    {
        //{ AudioID::Click, "Resource/Audio/Click.wav" },
        //{ AudioID::BGM,   "Resource/Audio/MainBGM.wav" }
    };

    const std::unordered_map<MusicID, std::filesystem::path> AssetHub::BuiltinMusicPaths =
    {
        { MusicID::Mus_BackGround, "Resource/Audio/BackgroundMusic.ogg"}
    };

    const std::unordered_map<FontID, std::pair<std::filesystem::path, int>> AssetHub::BuiltinFontPaths =
    {
        //{ FontID::Click, { "Resource/Fonts/Default.ttf", 24 } },
        //{ FontID::BGM,   { "Resource/Fonts/Title.ttf", 36 } }
    };

    bool AssetHub::Initialize(const std::vector<Online::Config::AnimationInfo>&  animationInfo,
        const std::vector<Online::TileEdit::TileMap>& tileMapInfo)
    {
        if (!std::filesystem::exists(Online::Core::GetExeDir() + "Resource")) { throw std::runtime_error("缺失资源文件"); }

        int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
        if ((IMG_Init(imgFlags) & imgFlags) != imgFlags)
        {
            Online::Log::Error(std::string("[AssetHub] IMG_Init failed: ") + IMG_GetError());
            return false;
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        {
            Online::Log::Warning(std::string("[AssetHub] Mix_OpenAudio failed: ") + Mix_GetError());
            SDL_Quit();
            return false;
        }

        int mixFlags = MIX_INIT_OGG | MIX_INIT_MP3 | MIX_INIT_FLAC;
        if ((Mix_Init(mixFlags) & mixFlags) != mixFlags)
        {
            Online::Log::Warning(std::string("[AssetHub] audio formats are not initialized: ") + Mix_GetError());
            return false;
        }

        if (TTF_Init() == -1)
        {
            Online::Log::Error(std::string("[AssetHub] TTF_Init failed: ") + TTF_GetError());
            IMG_Quit();
            Mix_CloseAudio();
            return false;
        }

        for (auto& tex : textures) tex = nullptr;
        for (auto& audio : sounds) audio = nullptr;
        for (auto& font : fonts) font = nullptr;
        for (auto& font : anims) font = nullptr;

        for (auto& flag : textureReady) flag.store(false, std::memory_order_release);
        for (auto& flag : soundReady) flag.store(false, std::memory_order_release);
        for (auto& flag : fontReady) flag.store(false, std::memory_order_release);
        for (auto& flag : animReady) flag.store(false, std::memory_order_release);

        isRunning.store(true, std::memory_order_release);


        animsPool.reserve(animationInfo.size() + 10);
        LoadHardAsset();
        LoadBuiltinAssets();
        LoadAnimationClipAssets(animationInfo);
		LoadTileMapAssets(tileMapInfo);

        isRunning.store(true, std::memory_order_release);

        loaderThread = Online::Thread::RegisterThread("Asset", &Online::Asset::AssetHub::BootstrapLogThread, this, nullptr);
        return true;
    }
    void AssetHub::Release()
    {
        isRunning.store(false, std::memory_order_release);

        Online::Thread::UnregisterThread(loaderThread);

        ClearAllResources();

        requestQueue.PopAll();
        resultQueue.PopAll();

        TTF_Quit();
        Mix_CloseAudio();
        Mix_Quit();
        IMG_Quit();

    }

    SDL_Texture* AssetHub::GetTexture(TextureID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(TextureID::Count))
        {
            Online::Log::Error("[AssetHub] Overflow Invalid TextureID: " + Online::Asset::TextureIDToString(id));
            return nullptr;
        }
        if (textures[idx] == nullptr)
        {
            Online::Log::Error("[AssetHub] Invalid TextureID: " + Online::Asset::TextureIDToString(id));
        }

        return textures[idx];
    }
    Mix_Chunk* AssetHub::GetSound(SoundID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(SoundID::Count))
        {
            Online::Log::Error("[AssetHub] Invalid AudioID: " + std::to_string(idx));
            return nullptr;
        }
        return sounds[idx];
    }
    Mix_Music* AssetHub::GetMusic(MusicID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(MusicID::Count))
        {
            Online::Log::Error("[AssetHub] Invalid AudioID: " + std::to_string(idx));
            return nullptr;
        }
        return musics[idx];
    }
    TTF_Font* AssetHub::GetFont(FontID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(FontID::Count))
        {
            Online::Log::Error("[AssetHub] Invalid FontID: " + std::to_string(idx));
            return nullptr;
        }
        return fonts[idx];
    }
    AnimationClip* AssetHub::GetAnim(AnimationClipID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(AnimationClipID::Count))
        {
            Online::Log::Error("[AssetHub] Invalid AnimationClipID: " + std::to_string(idx));
            return nullptr;
        }
        return anims[idx];
    }

    bool AssetHub::IsTextureReady(TextureID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(TextureID::Count)) return false;
        return textureReady[idx].load(std::memory_order_acquire);
    }
    bool AssetHub::IsSoundReady(SoundID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(SoundID::Count)) return false;
        return soundReady[idx].load(std::memory_order_acquire);
    }
    bool AssetHub::IsMusicReady(MusicID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(MusicID::Count)) return false;
        return musicReady[idx].load(std::memory_order_acquire);
    }
    bool AssetHub::IsFontReady(FontID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(FontID::Count)) return false;
        return fontReady[idx].load(std::memory_order_acquire);
    }
    bool AssetHub::IsAnimReady(Online::Asset::AnimationClipID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(Online::Asset::AnimationClipID::Count)) return false;
        return animReady[idx].load(std::memory_order_acquire);
    }

    void AssetHub::LoadTextureAsync(TextureID id, const std::filesystem::path& path)
    {
        LoadRequest req;
        req.type = LoadRequestType::Texture;
        req.id = id;

        std::filesystem::path fullPath = path.is_absolute()
            ? path
            : std::filesystem::path(Online::Core::GetExeDir()) / path;
        req.path = fullPath;

        requestQueue.Push(req);
    }
    void AssetHub::LoadSoundAsync(SoundID id, const std::filesystem::path& path)
    {
        LoadRequest req;
        req.type = LoadRequestType::Sound;
        req.id = id;

        std::filesystem::path fullPath = path.is_absolute()
            ? path
            : std::filesystem::path(Online::Core::GetExeDir()) / path;

        req.path = fullPath;

        requestQueue.Push(req);
    }
    void AssetHub::LoadFontAsync(FontID id, const std::filesystem::path& path, int fontSize)
    {
        LoadRequest req;
        req.type = LoadRequestType::Font;
        req.id = id;
        req.fontSize = fontSize;

        std::filesystem::path fullPath = path.is_absolute()
            ? path
            : std::filesystem::path(Online::Core::GetExeDir()) / path;

        req.path = fullPath;
        
        requestQueue.Push(req);
    }

    void AssetHub::SetRenderer(SDL_Renderer* renderer)
    {
        sdlRenderer = renderer;
    }
    void AssetHub::SyncLoadedAssets()
    {
        if (!sdlRenderer)
        {
            Online::Log::Warning("[AssetHub] SyncLoadedAssets failed: SDL_Renderer not bound");
            return;
        }

        auto resultQueue = this->resultQueue.PopAll();
        while (!resultQueue.empty())
        {
            auto result = std::move(resultQueue.front());
            resultQueue.pop();

            std::visit([this](auto&& res)
                {
                    using T = std::decay_t<decltype(res)>;

                    if constexpr (std::is_same_v<T, TextureLoadResult>)
                    {
                        uint8_t idx = static_cast<uint8_t>(res.id);

                        if (textures[idx])
                        {
                            SDL_DestroyTexture(textures[idx]);
                            textures[idx] = nullptr;
                        }

                        if (!res.success || !res.texture)
                        {
                            Online::Log::Error("[AssetHub] Texture load failed, ID: " + std::to_string(idx));
                            textureReady[idx].store(false, std::memory_order_release);
                            return;
                        }

                        textures[idx] = SDL_CreateTextureFromSurface(sdlRenderer, res.texture);
                        if (!textures[idx])
                        {
                            Online::Log::Error(std::string("[AssetHub] Create texture failed: ") + SDL_GetError());
                            textureReady[idx].store(false, std::memory_order_release);
                        }
                        else
                        {
                            textureReady[idx].store(true, std::memory_order_release);
                        }

                        SDL_FreeSurface(res.texture);
                    }
                    else if constexpr (std::is_same_v<T, SoundLoadResult>)
                    {
                        uint8_t idx = static_cast<uint8_t>(res.id);
                        if (sounds[idx])
                        {
                            Mix_FreeChunk(sounds[idx]);
                            sounds[idx] = nullptr;
                        }

                        if (res.success && res.chunk)
                        {
                            sounds[idx] = res.chunk;
                            soundReady[idx].store(true, std::memory_order_release);
                            Online::Log::Info("[AssetHub] Audio ready, ID: " + std::to_string(idx));
                        }
                        else
                        {
                            Online::Log::Error("[AssetHub] Audio load failed, ID: " + std::to_string(idx));
                            soundReady[idx].store(false, std::memory_order_release);
                        }
                    }
                    else if constexpr (std::is_same_v<T, FontLoadResult>)
                    {
                        uint8_t idx = static_cast<uint8_t>(res.id);
                        if (fonts[idx])
                        {
                            TTF_CloseFont(fonts[idx]);
                            fonts[idx] = nullptr;
                        }

                        if (res.success && res.font)
                        {
                            fonts[idx] = res.font;
                            fontReady[idx].store(true, std::memory_order_release);
                            Online::Log::Info("[AssetHub] Font ready, ID: " + std::to_string(idx));
                        }
                        else
                        {
                            Online::Log::Error("[AssetHub] Font load failed, ID: " + std::to_string(idx));
                            fontReady[idx].store(false, std::memory_order_release);
                        }
                    }
                }, result);
        }
    }
    void AssetHub::InitOffScreen()
    {
        if (!sdlRenderer)
        {
            Online::Log::Warning("[AssetHub] SyncLoadedAssets failed: SDL_Renderer not bound");
            return;
        }

        textureReady[static_cast<int>(Online::Asset::TextureID::Tex_WindowBuffer)].store(true, std::memory_order_release);

        textures[static_cast<int>(Online::Asset::TextureID::Tex_BackBuffer_1)] = SDL_CreateTexture(sdlRenderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            1280, 720);

        textureSizes[static_cast<int>(Online::Asset::TextureID::Tex_BackBuffer_1)] = { 1280,720 };

        textureReady[static_cast<int>(Online::Asset::TextureID::Tex_BackBuffer_1)].store(true, std::memory_order_release);

        textures[static_cast<int>(Online::Asset::TextureID::Tex_BackBuffer_2)] = SDL_CreateTexture(sdlRenderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            1280, 720);

        textureSizes[static_cast<int>(Online::Asset::TextureID::Tex_BackBuffer_2)] = { 1280,720 };

        textureReady[static_cast<int>(Online::Asset::TextureID::Tex_BackBuffer_2)].store(true, std::memory_order_release);
    }

    bool AssetHub::CreateOrResizeRenderTarget(TextureID id, int width, int height)
    {
        if (!sdlRenderer)
        {
            Online::Log::Warning("[AssetHub] SyncLoadedAssets failed: SDL_Renderer not bound");
            return false;
        }
        uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= textures.size()) return false;

        if (textures[idx] && textureSizes[idx].x == width && textureSizes[idx].y == height)
            return true;

        if (textures[idx])
            SDL_DestroyTexture(textures[idx]);

        textures[idx] = SDL_CreateTexture(sdlRenderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            width, height);
        if (!textures[idx])
        {
            Online::Log::Error("[AssetHub] CreateRenderTarget failed: " + std::string(SDL_GetError()));
            textureReady[idx].store(false, std::memory_order_release);
            return false;
        }

        SDL_SetTextureBlendMode(textures[idx], SDL_BLENDMODE_BLEND);
        textureSizes[idx] = { width, height };
        textureReady[idx].store(true, std::memory_order_release);
        return true;
    }

    glm::ivec2 AssetHub::GetTextureSize(TextureID id) const
    {
        uint8_t idx = static_cast<uint8_t>(id);

        if (idx >= static_cast<uint8_t>(TextureID::Count))
        {
            Online::Log::Warning("[AssetHub] GetTextureSize: Invalid TextureID -> " + std::to_string(idx));
            return { 0, 0 };
        }

        return textureSizes[idx];
    }

    void AssetHub::LoadHardAsset()
    {
#pragma region Texture
        SDL_Surface* bg_surface = nullptr;
        SDL_Surface* bar_surface = nullptr;
        SDL_Surface* run_surface = nullptr;
        SDL_Surface* flag_surface = nullptr;
        SDL_Surface* loading_surface = nullptr;

        SDL_RWops* rw_bg = SDL_RWFromConstMem(background_png_data, background_png_data_len);
        bg_surface = IMG_Load_RW(rw_bg, 1);
        SDL_RWops* rw_bar = SDL_RWFromConstMem(progressbar_png_data, progressbar_png_data_len);
        bar_surface = IMG_Load_RW(rw_bar, 1);
        SDL_RWops* rw_run = SDL_RWFromConstMem(progressrun_png_data, progressrun_png_data_len);
        run_surface = IMG_Load_RW(rw_run, 1);
        SDL_RWops* rw_flag = SDL_RWFromConstMem(flag_png_data, flag_png_data_len);
        flag_surface = IMG_Load_RW(rw_flag, 1);
        SDL_RWops* rw_loading = SDL_RWFromConstMem(loading_png_data, loading_png_data_len);
        loading_surface = IMG_Load_RW(rw_loading, 1);
        if (!rw_bg || !rw_bar || !rw_run || !rw_flag || !rw_loading)
            throw std::runtime_error("内置资源加载失败");

        TextureLoadResult barRes = LoadSurfaceInternal(static_cast<Asset::TextureID>(Asset::TextureID::Tex_ProgressBar), bar_surface);
        this->resultQueue.Push(barRes);
        TextureLoadResult bgRes = LoadSurfaceInternal(static_cast<Asset::TextureID>(Asset::TextureID::Tex_ProgressBar_Background), bg_surface);
        this->resultQueue.Push(bgRes);
        TextureLoadResult runRes = LoadSurfaceInternal(static_cast<Asset::TextureID>(Asset::TextureID::Tex_ProgressRun), run_surface);
        this->resultQueue.Push(runRes);
        TextureLoadResult flagRes = LoadSurfaceInternal(static_cast<Asset::TextureID>(Asset::TextureID::Tex_Flag), flag_surface);
        this->resultQueue.Push(flagRes);
        TextureLoadResult loadingRes = LoadSurfaceInternal(static_cast<Asset::TextureID>(Asset::TextureID::Tex_Loading), loading_surface);
        this->resultQueue.Push(loadingRes);
#pragma endregion

#pragma region Animation
        animsPool.clear();

        std::vector<int> progressRunFrmas = { 0,1,2,3,4,5 };
        animsPool.emplace_back("ProgressRun", 6, 1, 12, true, progressRunFrmas);
        AnimationClip& ProgressRunClip = animsPool.back();
        uint8_t ProgressRunClipID = static_cast<uint8_t>(Asset::AnimationClipID::Anim_ProgressRun);
        anims[ProgressRunClipID] = &ProgressRunClip;
        animReady[ProgressRunClipID].store(true, std::memory_order_release);

        std::vector<int> FlagFrmas = { 0,1,2,3,4,5 };
        animsPool.emplace_back("Flag", 6, 1, 12, true, FlagFrmas);
        AnimationClip& FlagClip = animsPool.back();
        uint8_t FlagClipID = static_cast<uint8_t>(Asset::AnimationClipID::Anim_Flag);
        anims[FlagClipID] = &FlagClip;
        animReady[FlagClipID].store(true, std::memory_order_release);

        std::vector<int> LoadingFrmas = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 };
        animsPool.emplace_back("Loading", 15, 1, 12, true, LoadingFrmas);
        AnimationClip& LoadingClip = animsPool.back();
        uint8_t LoadingClipID = static_cast<uint8_t>(Asset::AnimationClipID::Anim_Loading);
        anims[LoadingClipID] = &LoadingClip;
        animReady[LoadingClipID].store(true, std::memory_order_release);
#pragma endregion

#pragma region OGG
        Mix_Music* bk_music = nullptr;
        Mix_Music* kb_music = nullptr;

        SDL_RWops* rw_bgmusic = SDL_RWFromConstMem(bk_ogg_data, bk_ogg_data_len);
        SDL_RWops* rw_kbmusic = SDL_RWFromConstMem(kb_ogg_data, kb_ogg_data_len);
        bk_music = Mix_LoadMUS_RW(rw_bgmusic, 1);
        kb_music = Mix_LoadMUS_RW(rw_kbmusic, 1);
        uint8_t MusicID = static_cast<uint8_t>(Asset::MusicID::Mus_BackGround);
        if (!bk_music || !kb_music) { throw std::runtime_error("内置音频资源加载失败"); }
		musics[MusicID] = bk_music;
		musicReady[MusicID].store(true, std::memory_order_release);
        MusicID = static_cast<uint8_t>(Asset::MusicID::Mus_Man);
        musics[MusicID] = kb_music;
        musicReady[MusicID].store(true, std::memory_order_release);
#pragma endregion

    }
    inline bool AssetHub::IsAllAssetsLoaded()
    {
        if(isLoadedBuiltinAssets.load() && isLoadedAnimationsAssets.load() && isLoadedTileMapAssets.load())
        {
            return true;
		}
        return false;
    }
    void AssetHub::LoadBuiltinAssets()
    {
        for (const auto& [id, path] : BuiltinTexturePaths)
            resultQueue.Push((LoadTextureInternal(id, path.is_absolute()
                ? path
                : std::filesystem::path(Online::Core::GetExeDir()) / path)));

        for (const auto& [id, path] : BuiltinSoundPaths)
            resultQueue.Push((LoadSoundInternal(id, path.is_absolute()
                ? path
                : std::filesystem::path(Online::Core::GetExeDir()) / path)));

        for (const auto& [id, pathSize] : BuiltinFontPaths)
            resultQueue.Push((LoadFontInternal(id, pathSize.first.is_absolute()
                ? pathSize.first
                : std::filesystem::path(Online::Core::GetExeDir()) / pathSize.first, pathSize.second)));


        isLoadedBuiltinAssets.store(true, std::memory_order_release);

        if (IsAllAssetsLoaded())
        {
            isRunning.store(true, std::memory_order_release);
        }
        Online::Task::PostJob([this]()
            {


            }, "Load Buildin Asset");
    }
    void AssetHub::LoadAnimationClipAssets(const std::vector<Online::Config::AnimationInfo>& animationInfo)
    {
        Online::Task::PostJob([this, &animationInfo]()
            {
                for (auto& anim : animationInfo)
                {
                    animsPool.emplace_back(anim.name, anim.grid[0], anim.grid[1], anim.frameRate, anim.looping, anim.frames);
                    AnimationClip& clip = animsPool.back();
                    uint8_t clipID = static_cast<uint8_t>(StringToAnimationClipID(Online::Core::Prepend("Anim_", anim.name)));
                    anims[clipID] = &clip;
                    animReady[clipID].store(true, std::memory_order_release);
                }

                isLoadedAnimationsAssets.store(true, std::memory_order_release);

                if (IsAllAssetsLoaded())
                {
                    isRunning.store(true, std::memory_order_release);
                }

			}, "Load AnimationClip Asset");
    }
    void AssetHub::LoadTileMapAssets(const std::vector<Online::TileEdit::TileMap>& tileMapInfo)
    {
        Online::Task::PostJob([this, &tileMapInfo]()
            {


			}, "Load TileMap Asset");

        const uint8_t tilesetIdx = static_cast<uint8_t>(Online::Asset::TextureID::Tex_Tileset);

        uint8_t idx = static_cast<uint8_t>(TextureID::Tex_TileMap1);
        for (auto& tileMap : tileMapInfo)
        {
            if (idx > static_cast<uint8_t>(TextureID::Tex_TileMap4))
            {
                break;
            }
            TextureLoadResult res = LoadTileMapTextureInternal(static_cast<Asset::TextureID>(idx), tileMap);
            this->resultQueue.Push(res);
            idx++;
        }

        isLoadedTileMapAssets.store(true, std::memory_order_release);

        if (IsAllAssetsLoaded())
        {
            isRunning.store(true, std::memory_order_release);
        }
    }
    void AssetHub::LoaderThreadFunc()
    {
        while (isRunning.load(std::memory_order_acquire))
        {
            LoadRequest req;

            if (!requestQueue.Pop(req))
            {
                if (!isRunning.load(std::memory_order_acquire))
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            switch (req.type)
            {
            case LoadRequestType::Texture:
                resultQueue.Push(LoadTextureInternal(std::get<TextureID>(req.id), req.path));
                break;
            case LoadRequestType::Sound:
                resultQueue.Push(LoadSoundInternal(std::get<SoundID>(req.id), req.path));
                break;
            case LoadRequestType::Font:
                resultQueue.Push(LoadFontInternal(std::get<FontID>(req.id), req.path, req.fontSize));
                break;
            default:
                Online::Log::Error("[AssetHub] Unknown load request type");
                break;
            }
        }
    }

    TextureLoadResult AssetHub::LoadTextureInternal(TextureID id, const std::filesystem::path& path)
    {
        TextureLoadResult res;
        res.id = id;
        res.success = false;

        if (!std::filesystem::exists(path))
        {
            Online::Log::Error("[AssetHub] Texture file not found: " + path.string());
            return res;
        }

        SDL_Surface* surface = IMG_Load(path.string().c_str());
        if (!surface)
        {
            Online::Log::Error(std::string("[AssetHub] IMG_Load failed: ") + IMG_GetError() + Asset::TextureIDToString(id) + ", Path: " + path.string());
            return res;
        }

        uint8_t idx = static_cast<uint8_t>(id);
        textureSizes[idx] = { surface->w, surface->h };

        res.texture = surface;
        res.width = surface->w;
        res.height = surface->h;
        res.success = true;
        return res;
    }
    SoundLoadResult AssetHub::LoadSoundInternal(SoundID id, const std::filesystem::path& path)
    {
        SoundLoadResult res;
        res.id = id;
        res.success = false;

        if (!std::filesystem::exists(path))
        {
            Online::Log::Error("[AssetHub] Audio file not found: " + path.string());
            return res;
        }

        res.chunk = Mix_LoadWAV(path.string().c_str());
        res.success = (res.chunk != nullptr);
        if (!res.success)
        {
            Online::Log::Error(std::string("[AssetHub] Mix_LoadWAV failed: ") + Mix_GetError() + ", Path: " + path.string());
        }
        return res;
    }
    SoundLoadResult AssetHub::LoadMusicInternal(MusicID id, const std::filesystem::path& path)
    {
        return SoundLoadResult();
    }
    FontLoadResult AssetHub::LoadFontInternal(FontID id, const std::filesystem::path& path, int fontSize)
    {
        FontLoadResult res;
        res.id = id;
        res.success = false;

        if (!std::filesystem::exists(path))
        {
            Online::Log::Error("[AssetHub] Font file not found: " + path.string());
            return res;
        }

        res.font = TTF_OpenFont(path.string().c_str(), fontSize);
        res.success = (res.font != nullptr);
        if (!res.success)
        {
            Online::Log::Error(std::string("[AssetHub] TTF_OpenFont failed: ") + TTF_GetError() + ", Path: " + path.string());
        }
        return res;
    }
    TextureLoadResult AssetHub::LoadTileMapTextureInternal(TextureID id, const Online::TileEdit::TileMap& tilemap)
    {
        TextureLoadResult res;
        res.id = id;
        res.success = false;

        TextureLoadResult tilesetResult = LoadTextureInternal(TextureID::Tex_Tileset, BuiltinTexturePaths.at(TextureID::Tex_Tileset));
        if (!tilesetResult.success || !tilesetResult.texture)
        {
            Online::Log::Error("[AssetHub] 瓦片集加载失败，无法生成地图纹理");
            return res;
        }

        SDL_Surface* surface = tilemap.CreateMapSurface(tilesetResult.texture);
        if (!surface)
        {
            Online::Log::Error("[AssetHub] CreateMapSurface 生成地图Surface失败");
            SDL_FreeSurface(tilesetResult.texture);
            return res;
        }

        SDL_FreeSurface(tilesetResult.texture);

        uint8_t idx = static_cast<uint8_t>(id);
        textureSizes[idx] = { surface->w, surface->h };

        res.texture = surface;
        res.width = surface->w;
        res.height = surface->h;
        res.success = true;
        return res;
    }
    TextureLoadResult AssetHub::LoadSurfaceInternal(TextureID id, SDL_Surface* sdl_surface)
    {
        TextureLoadResult res;
        res.id = id;
        res.success = false;

        SDL_Surface* surface = sdl_surface;
        if (!surface)
        {
            Online::Log::Error(std::string("[AssetHub] Surface_Load failed: ") + IMG_GetError() + Asset::TextureIDToString(id));
            return res;
        }

        uint8_t idx = static_cast<uint8_t>(id);
        textureSizes[idx] = { surface->w, surface->h };

        res.texture = surface;
        res.width = surface->w;
        res.height = surface->h;
        res.success = true;
        return res;
    }
    bool AssetHub::SaveTextureToPNG(SDL_Texture* texture)
    {
        int w, h;
        if (SDL_QueryTexture(texture, nullptr, nullptr, &w, &h) != 0)
            return false;

        SDL_Texture* target = SDL_CreateTexture(
            sdlRenderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            w, h
        );
        if (!target)
            return false;

        SDL_SetRenderTarget(sdlRenderer, target);
        SDL_RenderCopy(sdlRenderer, texture, nullptr, nullptr);

        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA32);
        if (!surface)
        {
            SDL_DestroyTexture(target);
            return false;
        }

        int ret = SDL_RenderReadPixels(
            sdlRenderer,
            nullptr,
            SDL_PIXELFORMAT_RGBA32,
            surface->pixels,
            surface->pitch
        );

        SDL_SetRenderTarget(sdlRenderer, nullptr);
        SDL_DestroyTexture(target);

        if (ret != 0)
        {
            SDL_FreeSurface(surface);
            return false;
        }

        static std::atomic<uint16_t> count = 0;
        std::string filename = "test" + std::to_string(count.fetch_add(1)) + ".png";
        std::filesystem::path savePath = std::filesystem::path(Online::Core::GetExeDir()) / filename;
        std::string jobName = "SavePNG " + filename;

        Online::Task::PostJob([surface, savePath]()
            {
                IMG_SavePNG(surface, savePath.string().c_str());
                SDL_FreeSurface(surface);
            }, jobName.c_str());

        return true;
    }
    bool AssetHub::SaveWindowScreenshot(const std::filesystem::path& basename)
    {
        if (!sdlRenderer)
        {
            Online::Log::Error("[AssetHub] SaveWindowScreenshot failed: SDL_Renderer not bound");
            return false;
        }

        int width, height;
        if (SDL_GetRendererOutputSize(sdlRenderer, &width, &height) != 0)
        {
            Online::Log::Error(std::string("[AssetHub] SDL_GetRendererOutputSize failed: ") + SDL_GetError());
            return false;
        }

        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
            0,
            width,
            height,
            32,
            SDL_PIXELFORMAT_RGBA32
        );
        if (!surface)
        {
            Online::Log::Error(std::string("[AssetHub] SDL_CreateRGBSurfaceWithFormat failed: ") + SDL_GetError());
            return false;
        }

        SDL_SetRenderTarget(sdlRenderer, nullptr);
        int ret = SDL_RenderReadPixels(
            sdlRenderer,
            nullptr,
            SDL_PIXELFORMAT_RGBA32,
            surface->pixels,
            surface->pitch
        );

        if (ret != 0)
        {
            Online::Log::Error(std::string("[AssetHub] SDL_RenderReadPixels failed: ") + SDL_GetError());
            SDL_FreeSurface(surface);
            return false;
        }

        ImageSaveRequest req;
        req.surface = surface;
        static std::atomic<int> screenshotCount = 0;
        int count = screenshotCount.fetch_add(1);
        std::string extension;
        switch (req.type)
        {
        case SaveRequestType::JPG: extension = ".jpg"; break;
        case SaveRequestType::BMP: extension = ".bmp"; break;
        case SaveRequestType::PNG: extension = ".png"; break;
        }
        std::string finalFileName = basename.string() + std::to_string(count) + extension;

        std::filesystem::path saveDir;
        std::string picturesDirStr = Online::Core::GetPicturesDir();
        if (!picturesDirStr.empty())
            saveDir = std::filesystem::path(picturesDirStr) / "Online";
        else 
            saveDir = std::filesystem::path(Online::Core::GetExeDir());
        std::error_code ec;
        std::filesystem::create_directories(saveDir, ec);
        if (ec) 
        { 
            Online::Log::Warning("[AssetHub] Failed to create save dir: " + ec.message()); 
            saveDir = std::filesystem::path(Online::Core::GetExeDir()); 
        }

        req.path = saveDir / finalFileName;
        std::string jobName = "Save Image " + req.path.filename().string();

        Online::Task::PostJob([req = std::move(req)]() mutable
            {
                switch (req.type)
                {
                case SaveRequestType::PNG:
                    IMG_SavePNG(req.surface, req.path.string().c_str());
                    break;
                case SaveRequestType::JPG:
                    IMG_SaveJPG(req.surface, req.path.string().c_str(),req.quality);
                    break;
                case SaveRequestType::BMP:
                    SDL_SaveBMP(req.surface, req.path.string().c_str());
                    break;
                }
                SDL_FreeSurface(req.surface);
            }, jobName.c_str());

        return true;
    }

    float AssetHub::GetAsstetLoadProgress()
    {
        const int totalTasks = 3;
        int completedTasks = 0;

        if (isLoadedBuiltinAssets.load()) completedTasks++;
        if (isLoadedAnimationsAssets.load()) completedTasks++;
        if (isLoadedTileMapAssets.load()) completedTasks++;

        return static_cast<float>(completedTasks) / totalTasks;
    }

    void AssetHub::ClearAllResources()
    {
        for (uint8_t i = 0; i < textures.size(); ++i)
        {
            if (textures[i])
            {
                SDL_DestroyTexture(textures[i]);
                textures[i] = nullptr;
            }
            textureReady[i].store(false, std::memory_order_release);
        }
        for (uint8_t i = 0; i < sounds.size(); ++i)
        {
            if (sounds[i])
            {
                Mix_FreeChunk(sounds[i]);
                sounds[i] = nullptr;
            }
            soundReady[i].store(false, std::memory_order_release);
        }
        for (uint8_t i = 0; i < fonts.size(); ++i)
        {
            if (fonts[i])
            {
                TTF_CloseFont(fonts[i]);
                fonts[i] = nullptr;
            }
            fontReady[i].store(false, std::memory_order_release);
        }
    }
}