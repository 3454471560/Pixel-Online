#pragma once
#include <Core/Color/Color.h>
#include <Asset/Common/ID/TextureID.h>

#include <SDL.h>
#include <glm.hpp>
namespace Online::Render
{
	struct PostProcessCommand
	{
		Online::Asset::TextureID SourceTexture = Online::Asset::TextureID::Tex_WindowBuffer;
		Online::Core::Color Color = Online::Core::Color::White;
		uint8_t Alpha = 255;
		SDL_BlendMode BlendMode = SDL_BLENDMODE_BLEND;
		SDL_ScaleMode ScaleMode = SDL_ScaleModeLinear;
		glm::ivec2 DrawOffset = { 0, 0 };
		glm::ivec2 DrawSize = { 0, 0 };
		float Rotation = 0.0f;
		SDL_FPoint RotatePivot = { 0.5f, 0.5f };
		SDL_RendererFlip Flip = SDL_FLIP_NONE;

		PostProcessCommand() = default;
		PostProcessCommand(Online::Asset::TextureID sourceTexture)
			: SourceTexture(sourceTexture)
		{
		}
	};
}