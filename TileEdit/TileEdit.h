#pragma once
#include <Core/Singleton/Singleton.h>
#include <TileEdit/Common/TileMap.h>

#include<string>
#include<vector>

namespace Online::TileEdit { class TileEdit; }

namespace Online::TileEdit
{
    class TileEdit final : public Online::Core::Singleton<TileEdit>
    {
        friend class Online::Core::Singleton<TileEdit>;

    private:
        TileEdit() = default;
        ~TileEdit() = default;

    public:
        TileEdit(const TileEdit&) = delete;
        TileEdit& operator=(const TileEdit&) = delete;
        TileEdit(TileEdit&&) = delete;
        TileEdit& operator=(TileEdit&&) = delete;
    public:

    public:
        int Execute();
        void Terminate();

    private:
        bool Initialize();
        bool IsRunning() const;
        void BeginFrame();
        void ImGui();
        void Update();
        void LateUpdate();
        void Render();
        void EndFrame();
        void FrameSync();
        void Release();

    public:
        void Save();
        void SaveAs();
        void SaveCurrentView();

        void Open();
        void NewBuilt();

        bool IsTileEmpty(int tileX, int tileY);
    private:
#pragma region while
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Event event;
        double freq;
        Uint64 lastCounter;
        Uint64 startCounter;
        bool isRunning = true;
        double frame_time = 0.0;
#pragma endregion

#pragma region map
        SDL_Texture* tilesetTexture = nullptr;
        SDL_Surface* tilesetSurface = nullptr;
        int selectedTileId = -1;
        TileMap* tileMap = nullptr;
#pragma endregion

#pragma region imgui
        bool showMapCreat = false;
        bool OpenshowSaveWarn = false;
        bool NewshowSaveWarn = false;
		bool isFileOpen = false;
		bool isChanged = false;
		bool EditMode = false;
        bool showGrid = true;
        bool showBackgroundTiles = true;
        bool showTerrainTiles = true;
        bool showDecorationTiles = true;
        std::string mapWidthStr;
        std::string mapHeightStr;
		std::string mapFilePath;
        float scale = 1.6f;
        float mapScale = 1.6f;
        const int LAYER_BACKGROUND = 1;
        const int LAYER_TERRAIN = 2;
        const int LAYER_DECORATION = 4;
        int editLayer = LAYER_BACKGROUND;
#pragma endregion
    };
}

extern template class Online::Core::Singleton<Online::TileEdit::TileEdit>;