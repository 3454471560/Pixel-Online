#pragma once

#include <Core/StateFlags/StateFlags.h>
#include <Core/Color/Color.h>
#include <Core/Anchor/Anchor.h>
#include <Render/Common/RenderLayer.h>
#include <Render/Common/RenderQueue.h>
#include <Asset/Common/ID/TextureID.h>
#include <Asset/Common/ID/FontID.h>

#include <glm.hpp>
#include <SDL.h>
#include <cstdint>

namespace Online::Render
{
    struct RenderText
    {
        Online::Core::StateFlags<Online::Render::RenderLayer> LayerMask;
        RenderQueue RenderQueueType = RenderQueue::World;
        uint8_t        Depth = 0;
        uint8_t        DrawOrder = 0;
        Asset::FontID fontID;
        std::string text;
        glm::vec2 position;
        Core::Anchor Anchor;
        float scale = 1.0f;
        Online::Core::Color Color = Online::Core::Color::White;
        float Width = 0.f;
		float LetterSpacing = 0.f;

        SDL_FPoint  Pivot = { 0, 0 };
        float       Rotation = 0.0f;

        RenderText() = default;
    };
}