#pragma once
#include <Core/Color/Color.h>
#include <Render/Command/DrawCommand.h>
#include <Asset/Common/ID/TextureID.h>
#include <Asset/Common/FuncTable.h>

#include <SDL.h>

namespace Online::Render
{
    struct InstancedDrawCommand
    {
        SDL_Texture*     Texture = nullptr;
        SDL_Rect         SrcRect = { 0, 0, 0, 0 };
        SDL_FRect        DstRect = { 0.0f, 0.0f, 0.0f, 0.0f };
        float            Rotation = 0.0f;
        SDL_FPoint       Pivot = { 0.0f, 0.0f };
        SDL_RendererFlip Flip = SDL_FLIP_NONE;
        Online::Core::Color Color = Online::Core::Color::White;

        InstancedDrawCommand() = default;

        InstancedDrawCommand(const DrawCommand& other)
            : SrcRect(other.SrcRect)
            , DstRect(other.DstRect)
            , Rotation(other.Rotation)
            , Pivot(other.Pivot)
            , Flip(other.Flip) 
            , Color(other.Color)
        {
            this->Texture = Online::Asset::GetTexture(other.TextureID);
            if (SrcRect.w == 0.0f || SrcRect.h == 0.0f)
            {
                SDL_QueryTexture(Texture, nullptr, nullptr, &SrcRect.w, &SrcRect.h);
            }
            if(DstRect.w == 0.0f || DstRect.h == 0.0f)
            {
                DstRect.w = static_cast<float>(SrcRect.w);
                DstRect.h = static_cast<float>(SrcRect.h);
			}
        }

        InstancedDrawCommand& operator=(const DrawCommand& other) 
        {
            this->Texture = Online::Asset::GetTexture(other.TextureID);
            this->SrcRect = other.SrcRect;
            this->DstRect = other.DstRect;
            this->Rotation = other.Rotation;
            this->Pivot = other.Pivot;
            this->Flip = other.Flip;
            this->Color = other.Color;
            if (SrcRect.w == 0.0f || SrcRect.h == 0.0f)
            {
                SDL_QueryTexture(Texture, nullptr, nullptr, &SrcRect.w, &SrcRect.h);
            }
            if (DstRect.w == 0.0f || DstRect.h == 0.0f)
            {
                DstRect.w = static_cast<float>(SrcRect.w);
                DstRect.h = static_cast<float>(SrcRect.h);
            }
            return *this;
        }
    };
}
