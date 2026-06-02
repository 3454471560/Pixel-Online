#pragma once

#include<Core/Camera/CameraSnapshot.h>
#include<Render/Common/RenderTargetSnapshot.h>

#include<glm.hpp>

namespace Online::Render
{
	struct BeginPassCommand
	{
		Online::Core::CameraSnapshot CameraSnapshot;
		Online::Render::RenderTargetSnapshot RenderTarget;
	};
}
