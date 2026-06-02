#pragma once
#include <Core/Map/Tile.h>
#include <Core/Utils/File.h>

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include <glm.hpp>

#include<string>
#include<stdexcept>
#include<fstream>
#include<sstream>

namespace Online::TileEdit
{
    class TileMap
    {
    public:
        TileMap() = default;
        ~TileMap() = default;
        void serialize() const
        {
            if (mapWidth <= 0 || mapHeight <= 0 || tileMap.empty())
            {
                throw std::runtime_error("地图数据无效，无法保存！");
            }

            std::string fullPath = EnsureCsvExtension(Online::Core::GetExeDir() + "map");
            std::ofstream file(fullPath, std::ios::out | std::ios::trunc);
            if (!file.is_open())
            {
                throw std::runtime_error("保存文件失败\n（该文件可能已经被打开，无法编辑）: " + fullPath);
            }

            for (const auto& row : tileMap)
            {
                for (size_t i = 0; i < row.size(); ++i)
                {
                    const auto& tile = row[i];
                    file << tile.background << '\\' << tile.terrain << '\\' << tile.decoration;

                    if (i != row.size() - 1)
                    {
                        file << ',';
                    }
                }
                file << '\n';
            }

            file.close();
        }
        void serialize(const std::string& filepath) const
        {
            if (mapWidth <= 0 || mapHeight <= 0 || tileMap.empty())
            {
                throw std::runtime_error("地图数据无效，无法保存！");
            }

            std::string finalPath = EnsureCsvExtension(filepath);
            std::ofstream file(finalPath, std::ios::out | std::ios::trunc);
            if (!file.is_open())
            {
                throw std::runtime_error("保存文件失败\n（该文件可能已经被打开，无法编辑）: " + finalPath);
            }

            for (const auto& row : tileMap)
            {
                for (size_t i = 0; i < row.size(); ++i)
                {
                    const auto& tile = row[i];
                    file << tile.background << '\\' << tile.terrain << '\\' << tile.decoration;
                    if (i != row.size() - 1)
                    {
                        file << ',';
                    }
                }
                file << '\n';
            }
            file.close();
        }
        void deserialize(const std::string& filepath)
        {
            std::ifstream file(filepath);
            if (!file.is_open())
            {
                throw std::runtime_error("打开文件失败: " + filepath);
            }

            std::string line;
            std::vector<std::vector<Core::Tile>> temp_map;

            while (std::getline(file, line))
            {
                line = trimString(line);
                if (line.empty()) continue;

                std::stringstream ss(line);
                std::string tile_str;
                std::vector<Core::Tile> row;

                while (std::getline(ss, tile_str, ','))
                {
                    Core::Tile tile;
                    loadTileFromString(tile_str, tile);
                    row.push_back(tile);
                }

                if (!row.empty())
                {
                    temp_map.push_back(std::move(row));
                }
            }

            file.close();

            if (temp_map.empty() || temp_map[0].empty())
            {
                throw std::runtime_error("地图文件数据无效！");
            }

            tileMap = std::move(temp_map);
            mapHeight = (int)tileMap.size();
            mapWidth = (int)tileMap[0].size();
        }

    private:
        std::string trimString(const std::string& str)
        {
            size_t first = str.find_first_not_of(" \t");
            size_t last = str.find_last_not_of(" \t");
            if (first == std::string::npos || last == std::string::npos)
                return "";
            return str.substr(first, last - first + 1);
        }
        void loadTileFromString(const std::string& tile_str, Online::Core::Tile& tile)
        {
            std::string str_tidy = trimString(tile_str);

            std::string str_value;
            std::vector<int> values;
            std::stringstream ss(str_tidy);

            while (std::getline(ss, str_value, '\\'))
            {
                int value;
                try
                {
                    value = std::stoi(trimString(str_value));
                }
                catch (const std::exception&)
                {
                    value = -1;
                }

                values.push_back(value);
            }

            tile.background = (values.size() < 1 || values[0] < 0) ? -1 : values[0];
            tile.terrain = (values.size() < 2) ? -1 : values[1];
            tile.decoration = (values.size() < 3) ? -1 : values[2];

        }
        static std::string EnsureCsvExtension(const std::string& filepath)
        {
            static const std::string csv_ext = ".csv";

            if (filepath.empty())
                return csv_ext;

            bool has_extension = false;
            if (filepath.size() >= csv_ext.size())
            {
                std::string last_chars = filepath.substr(filepath.size() - csv_ext.size());
                std::transform(last_chars.begin(), last_chars.end(), last_chars.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                has_extension = (last_chars == csv_ext);
            }

            return has_extension ? filepath : filepath + csv_ext;
        }
        static std::string EnsurePngExtension(const std::string& filepath)
        {
            static const std::string png_ext = ".png";
            if (filepath.empty())
                return png_ext;

            bool has_extension = false;
            if (filepath.size() >= png_ext.size())
            {
                std::string last_chars = filepath.substr(filepath.size() - png_ext.size());
                std::transform(last_chars.begin(), last_chars.end(), last_chars.begin(),
                    [](unsigned char c) { return std::tolower(c); });
                has_extension = (last_chars == png_ext);
            }

            return has_extension ? filepath : filepath + png_ext;
        }
        SDL_Texture* CreateMapTextureUtil(SDL_Renderer* renderer, SDL_Texture* tilesetTexture, const bool* showLayers = nullptr)
        {
            if (!renderer || !tilesetTexture || tileMap.empty() || mapWidth <= 0 || mapHeight <= 0)
                return nullptr;

            const int tileSize = 32;
            const int textureWidth = mapWidth * tileSize;
            const int textureHeight = mapHeight * tileSize;

            SDL_Texture* mapTexture = SDL_CreateTexture(
                renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                textureWidth, textureHeight
            );
            if (!mapTexture) return nullptr;

            SDL_Texture* oldTarget = SDL_GetRenderTarget(renderer);
            SDL_SetRenderTarget(renderer, mapTexture);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            int tilesetW = 0, tilesetH = 0;
            SDL_QueryTexture(tilesetTexture, nullptr, nullptr, &tilesetW, &tilesetH);
            const int tilesPerRow = tilesetW / tileSize;

            const bool showBg = (showLayers == nullptr) ? true : showLayers[0];
            const bool showTerrain = (showLayers == nullptr) ? true : showLayers[1];
            const bool showDeco = (showLayers == nullptr) ? true : showLayers[2];

            for (int y = 0; y < mapHeight; ++y)
            {
                for (int x = 0; x < mapWidth; ++x)
                {
                    const auto& tile = tileMap[y][x];
                    const SDL_Rect dst = { x * tileSize, y * tileSize, tileSize, tileSize };

                    if (showBg && tile.background >= 0)
                    {
                        const int tx = tile.background % tilesPerRow;
                        const int ty = tile.background / tilesPerRow;
                        const SDL_Rect src = { tx * tileSize, ty * tileSize, tileSize, tileSize };
                        SDL_RenderCopy(renderer, tilesetTexture, &src, &dst);
                    }
                    if (showTerrain && tile.terrain >= 0)
                    {
                        const int tx = tile.terrain % tilesPerRow;
                        const int ty = tile.terrain / tilesPerRow;
                        const SDL_Rect src = { tx * tileSize, ty * tileSize, tileSize, tileSize };
                        SDL_RenderCopy(renderer, tilesetTexture, &src, &dst);
                    }
                    if (showDeco && tile.decoration >= 0)
                    {
                        const int tx = tile.decoration % tilesPerRow;
                        const int ty = tile.decoration / tilesPerRow;
                        const SDL_Rect src = { tx * tileSize, ty * tileSize, tileSize, tileSize };
                        SDL_RenderCopy(renderer, tilesetTexture, &src, &dst);
                    }
                }
            }

            SDL_SetRenderTarget(renderer, oldTarget);
            return mapTexture;
        }
        SDL_Surface* CreateMapSurfaceUtil(SDL_Surface* tilesetSurface, const bool* showLayers) const
        {
            if (!tilesetSurface || tileMap.empty() || mapWidth <= 0 || mapHeight <= 0)
                return nullptr;

            const int tileSize = 32;
            const int mapW = mapWidth * tileSize;
            const int mapH = mapHeight * tileSize;

            SDL_Surface* mapSurface = SDL_CreateRGBSurfaceWithFormat(
                0, mapW, mapH, 32, SDL_PIXELFORMAT_RGBA8888
            );
            if (!mapSurface)
                return nullptr;

            SDL_FillRect(mapSurface, nullptr, SDL_MapRGBA(mapSurface->format, 0, 0, 0, 0));

            SDL_Surface* convertedTileset = SDL_ConvertSurfaceFormat(tilesetSurface, SDL_PIXELFORMAT_RGBA8888, 0);
            if (!convertedTileset)
            {
                SDL_FreeSurface(mapSurface);
                return nullptr;
            }

            if (SDL_LockSurface(convertedTileset) != 0 || SDL_LockSurface(mapSurface) != 0)
            {
                SDL_UnlockSurface(convertedTileset);
                SDL_UnlockSurface(mapSurface);
                SDL_FreeSurface(convertedTileset);
                SDL_FreeSurface(mapSurface);
                return nullptr;
            }

            const Uint32* tilePixels = static_cast<Uint32*>(convertedTileset->pixels);
            Uint32* mapPixels = static_cast<Uint32*>(mapSurface->pixels);
            const int tilesetW = convertedTileset->w;
            const int tilesPerRow = tilesetW / tileSize;

            const bool showBg = (showLayers == nullptr) ? true : showLayers[0];
            const bool showTerrain = (showLayers == nullptr) ? true : showLayers[1];
            const bool showDeco = (showLayers == nullptr) ? true : showLayers[2];

            for (int y = 0; y < mapHeight; ++y)
            {
                if (y >= tileMap.size()) continue;
                const auto& row = tileMap[y];

                for (int x = 0; x < mapWidth; ++x)
                {
                    if (x >= row.size()) continue;
                    const auto& tile = row[x];

                    const int baseX = x * tileSize;
                    const int baseY = y * tileSize;

                    auto DrawTile = [=, &tilePixels, &mapPixels](int tileId)
                        {
                            if (tileId < 0) return;

                            const int tx = (tileId % tilesPerRow) * tileSize;
                            const int ty = (tileId / tilesPerRow) * tileSize;

                            if (tx < 0 || ty < 0 || tx + tileSize > tilesetW || ty + tileSize > convertedTileset->h)
                                return;

                            for (int dy = 0; dy < tileSize; ++dy)
                            {
                                for (int dx = 0; dx < tileSize; ++dx)
                                {
                                    const int srcIdx = (ty + dy) * tilesetW + (tx + dx);
                                    const int dstIdx = (baseY + dy) * mapW + (baseX + dx);
                                    const Uint32 color = tilePixels[srcIdx];

                                    if ((color >> 24) & 0xFF)
                                        mapPixels[dstIdx] = color;
                                }
                            }
                        };

                    if (showBg) DrawTile(tile.background);
                    if (showTerrain) DrawTile(tile.terrain);
                    if (showDeco) DrawTile(tile.decoration);
                }
            }

            SDL_UnlockSurface(mapSurface);
            SDL_UnlockSurface(convertedTileset);
            SDL_FreeSurface(convertedTileset);

            return mapSurface;
        }    public:
        void setMapWidth(int width) 
        {
            mapWidth = width; 
        }
        void setMapHeight(int height) 
        {
            mapHeight = height; 
        }
        void SetTileLayer(int x, int y, int layer, int value)
        {
            auto& tile = tileMap[y][x];
            switch (layer)
            {
            case 1: tile.background = value; break;
            case 2: tile.terrain = value; break;
            case 4: tile.decoration = value; break;
            }
        }

        void Clear()
        {
            tileMap.clear();
        }
    public:
        int& getMapWidth() 
        { 
            return mapWidth; 
        }
        int& getMapHeight() 
        { 
            return mapHeight; 
        }

        size_t GetWidth() const
        {
            return tileMap.empty() ? 0 : tileMap[0].size();
        }
        size_t GetHeight() const
        {
            return tileMap.size();
        }

        Core::Tile& GetTile(int x, int y)
        {
            return tileMap[y][x];
        }
        const Core::Tile& GetTile(int x, int y) const
        {
            return tileMap[y][x];
        }

        bool is_valid() const 
        {
            return !tileMap.empty() && !tileMap[0].empty();
        }

    public:
        void InitTileMap(int width, int height)
        {
            tileMap.resize(height);

            for (int y = 0; y < height; ++y)
            {
                tileMap[y].resize(width);
            }

            mapWidth = width;
            mapHeight = height;
        }
        void SaveAsPNG(const std::string& savePath, SDL_Renderer* renderer, SDL_Texture* tilesetTexture, float mapScale,
            bool showBackground, bool showTerrain, bool showDecoration)
        {
            if (!renderer || !tilesetTexture || tileMap.empty() || mapWidth <= 0 || mapHeight <= 0)
                return;

            std::string finalPath = EnsurePngExtension(savePath);

            const int tileSize = 32;
            const int scaledSize = static_cast<int>(std::round(tileSize * mapScale));
            const int imgW = mapWidth * scaledSize;
            const int imgH = mapHeight * scaledSize;

            SDL_Texture* targetTex = SDL_CreateTexture(
                renderer,
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_TARGET,
                imgW, imgH
            );
            if (!targetTex) return;

            SDL_SetRenderTarget(renderer, targetTex);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);

            int texW = 0, texH = 0;
            SDL_QueryTexture(tilesetTexture, nullptr, nullptr, &texW, &texH);
            const int tilesPerRow = texW / tileSize;

            for (int y = 0; y < mapHeight; ++y)
            {
                for (int x = 0; x < mapWidth; ++x)
                {
                    const auto& tile = tileMap[y][x];
                    const int posX = x * scaledSize;
                    const int posY = y * scaledSize;

                    SDL_Rect dstRect = { posX, posY, scaledSize, scaledSize };

                    if (showBackground && tile.background >= 0)
                    {
                        const int tx = tile.background % tilesPerRow;
                        const int ty = tile.background / tilesPerRow;
                        SDL_Rect srcRect = { tx * tileSize, ty * tileSize, tileSize, tileSize };
                        SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &dstRect);
                    }
                    if (showTerrain && tile.terrain >= 0)
                    {
                        const int tx = tile.terrain % tilesPerRow;
                        const int ty = tile.terrain / tilesPerRow;
                        SDL_Rect srcRect = { tx * tileSize, ty * tileSize, tileSize, tileSize };
                        SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &dstRect);
                    }
                    if (showDecoration && tile.decoration >= 0)
                    {
                        const int tx = tile.decoration % tilesPerRow;
                        const int ty = tile.decoration / tilesPerRow;
                        SDL_Rect srcRect = { tx * tileSize, ty * tileSize, tileSize, tileSize };
                        SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &dstRect);
                    }
                }
            }

            Uint32* pixels = new Uint32[imgW * imgH];
            SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_RGBA8888, pixels, imgW * sizeof(Uint32));

            SDL_Surface* saveSurface = SDL_CreateRGBSurfaceWithFormatFrom(
                pixels, imgW, imgH, 32,
                imgW * sizeof(Uint32),
                SDL_PIXELFORMAT_RGBA8888
            );

            if (saveSurface)
            {
                IMG_SavePNG(saveSurface, finalPath.c_str());
                SDL_FreeSurface(saveSurface);
            }

            delete[] pixels;
            SDL_DestroyTexture(targetTex);
            SDL_SetRenderTarget(renderer, nullptr);
        }
        SDL_Texture* CreateMapTexture(SDL_Renderer* renderer, SDL_Texture* tilesetTexture)
        {
            return CreateMapTextureUtil(renderer, tilesetTexture, nullptr);
        }
        SDL_Surface* CreateMapSurface(SDL_Surface* tilesetSurface) const 
        {
            return CreateMapSurfaceUtil(tilesetSurface, nullptr);
        }
        std::vector<glm::ivec2> GetTerrainTileIndices() const
        {
            std::vector<glm::ivec2> indices;
            for (int y = 0; y < (int)tileMap.size(); ++y)
            {
                for (int x = 0; x < (int)tileMap[y].size(); ++x)
                {
                    if (tileMap[y][x].terrain >= 0)
                        indices.push_back({ x, y });
                }
            }
            return indices;
        }
    private:
        int mapWidth = -1;
        int mapHeight = -1;
        int tileSize = 32;
        std::vector<std::vector<Core::Tile>> tileMap;
    };

}