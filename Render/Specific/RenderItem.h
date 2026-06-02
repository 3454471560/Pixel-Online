#pragma once

#include <Core/StateFlags/StateFlags.h>
#include <Core/Color/Color.h>
#include <Render/Common/RenderLayer.h>
#include <Render/Common/RenderQueue.h>
#include <Asset/Common/ID/TextureID.h>

#include <glm.hpp>
#include <SDL.h>
#include <cstdint>

namespace Online::Render
{
    struct RenderItem
    {
        SDL_Rect    SrcRect = { 0, 0, 0, 0 };
        SDL_FRect   DstRect = { 0, 0, 0, 0 };
        SDL_FPoint  Pivot = { 0, 0 };
        Online::Core::StateFlags<Online::Render::RenderLayer> LayerMask;
        Online::Asset::TextureID TextureID = Online::Asset::TextureID::Tex_Default;
        RenderQueue RenderQueueType = RenderQueue::World;
        float       Rotation = 0.0f;
        SDL_RendererFlip Flip = SDL_FLIP_NONE;
        uint8_t DrawOrder = 0;
        uint8_t Depth = 0;
        Online::Core::Color Color = Online::Core::Color::White;

        RenderItem() = default;

        explicit RenderItem(Online::Render::RenderLayer layerMask)
            : LayerMask(layerMask)
        {
        }

        RenderItem(
            Online::Render::RenderLayer layerMask,
            Online::Asset::TextureID textureID,
            RenderQueue queue,
            const SDL_Rect& srcRect,
            const SDL_FRect& dstRect,
            uint8_t drawOrder = 0,
            uint8_t depth = 0,
            float rotation = 0.0f,
            const SDL_FPoint& pivot = { 0,0 },
            SDL_RendererFlip flip = SDL_FLIP_NONE,
            const glm::vec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }
        )
            : LayerMask(layerMask)
            , TextureID(textureID)
            , RenderQueueType(queue)
            , DrawOrder(drawOrder)
            , Depth(depth)
            , SrcRect(srcRect)
            , DstRect(dstRect)
            , Rotation(rotation)
            , Pivot(pivot)
            , Flip(flip)
            , Color(color)
        {
        }
    };
}