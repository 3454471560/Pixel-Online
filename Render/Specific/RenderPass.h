#pragma once

#include<Core/StateFlags/StateFlags.h>
#include<Core/Camera/CameraSnapshot.h>
#include<Render/Common/RenderTargetSnapshot.h>
#include<Render/Common/PostProcessSetting.h>
#include<Render/Common/RenderLayer.h>

namespace Online::Render
{
	struct RenderPass
	{
		Online::Core::CameraSnapshot CameraSnapshot;
		Online::Render::RenderTargetSnapshot RenderTarget;
		Online::Render::PostProcessSetting PostProcessSetting;
		Online::Core::StateFlags<Online::Render::RenderLayer> CullingMask;

		RenderPass() = default;

		RenderPass(const Online::Core::CameraSnapshot& cameraSnapshot, const Online::Render::RenderTargetSnapshot& renderTargetSnapshot,const Online::Render::PostProcessSetting& setting, Online::Core::StateFlags<Online::Render::RenderLayer> cullingMask)
			:CameraSnapshot(cameraSnapshot), RenderTarget(renderTargetSnapshot), PostProcessSetting(setting), CullingMask(cullingMask) {}
	};
}