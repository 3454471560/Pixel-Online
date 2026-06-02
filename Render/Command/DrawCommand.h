#pragma once
#include <Core/Color/Color.h>
#include <Render/Common/RenderQueue.h>
#include <Asset/Common/ID/TextureID.h>
#include <Render/Specific/RenderItem.h>

#include <SDL.h>
#include <glm.hpp>
#include <algorithm>
#include <cstdint>
#include <cassert>

namespace Online::Render
{
    inline constexpr uint32_t MaxSortKey = 0xFFFFFFFF;
    inline constexpr uint32_t MinSortKey = 0;

    // RenderQueue(16) > Depth(4) > DrawOrder(4) > TextureID(8)
    inline constexpr uint32_t RENDER_QUEUE_SHIFT = 16;
    inline constexpr uint32_t DEPTH_SHIFT = 12;
    inline constexpr uint32_t DRAW_ORDER_SHIFT = 8;
    inline constexpr uint32_t TEXTURE_ID_SHIFT = 0;

    inline constexpr uint32_t RENDER_QUEUE_MASK = 0xFFFF;
    inline constexpr uint32_t DEPTH_MASK = 0x0F;
    inline constexpr uint32_t DRAW_ORDER_MASK = 0x0F;
    inline constexpr uint32_t TEXTURE_ID_MASK = 0xFF;

    inline constexpr uint32_t GenerateSortKey(
        RenderQueue renderQueue,
        uint8_t depth,
        uint8_t drawOrder,
        Online::Asset::TextureID textureID)
    {
        uint32_t key = 0;

        key |= (static_cast<uint32_t>(renderQueue) & RENDER_QUEUE_MASK) << RENDER_QUEUE_SHIFT;
        key |= (static_cast<uint32_t>(depth) & DEPTH_MASK) << DEPTH_SHIFT;
        key |= (static_cast<uint32_t>(drawOrder) & DRAW_ORDER_MASK) << DRAW_ORDER_SHIFT;
        key |= (static_cast<uint32_t>(textureID) & TEXTURE_ID_MASK) << TEXTURE_ID_SHIFT;

        assert(depth <= DEPTH_MASK && "Depth must be 0-15");
        return key;
    }

    struct DrawCommand
    {
        Online::Asset::TextureID TextureID = Online::Asset::TextureID::Tex_Default;
        uint32_t SortKey = MinSortKey;
        SDL_Rect SrcRect = { 0, 0, 0, 0 };
        SDL_FRect DstRect = { 0.0f, 0.0f, 0.0f, 0.0f };
        float Rotation = 0.0f;
        SDL_FPoint Pivot = { 0.0f, 0.0f };
        SDL_RendererFlip Flip = SDL_FLIP_NONE;
        uint8_t Depth = 0;
        Online::Core::Color Color = Online::Core::Color::White;

        DrawCommand() = default;

        explicit DrawCommand(const RenderItem& item)
            : TextureID(item.TextureID)
            , SortKey(GenerateSortKey(
                item.RenderQueueType,
                item.Depth,
                item.DrawOrder,
                item.TextureID))
            , SrcRect(item.SrcRect)
            , DstRect(item.DstRect)
            , Rotation(item.Rotation)
            , Pivot(item.Pivot)
            , Flip(item.Flip)
            , Depth(item.Depth)
            , Color(item.Color)
        {
        }
    };
}