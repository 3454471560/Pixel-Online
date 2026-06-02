#pragma once
#include <Core/Color/Color.h>
#include <Asset/Common/ID/TextureID.h>
#include <Render/Command/PostProcessCommand.h>

#include <SDL.h>

namespace Online::Render
{
	struct PostProcessSetting
	{
		Online::Core::Color Color = Online::Core::Color::White;
		uint8_t Alpha = 255;
		bool Blend = true;
		bool Linear = true;
		float Rotation = 0.0f;
		bool Flip = false;

		inline PostProcessCommand BuildProcessCommand() const
		{
			PostProcessCommand processcommand;
			processcommand.Color = Color;
			processcommand.Alpha = Alpha;
			processcommand.BlendMode = Blend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE;
			processcommand.ScaleMode = Linear ? SDL_ScaleModeLinear : SDL_ScaleModeNearest;
			processcommand.Rotation = Rotation;
			processcommand.Flip = Flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
			return processcommand;
		}
	};
}