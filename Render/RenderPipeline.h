#pragma once

#include<Core/Allocate/Allocate.h>
#include<Core/Color/Color.h>
#include<Context/Common/Module.h>
#include<Context/Context.h>
#include<Client/Context/ClientContext.h>
#include<Render/Specific/RenderPass.h>
#include<Render/Specific/RenderItem.h>

#include<glm.hpp>
#include<vector>

namespace Online::Render
{
	class RenderPipeline
	{
	public:
		struct Factory
		{
			friend class Online::Runtime::Module<RenderPipeline>;
		private:
			static RenderPipeline* Create()
			{
				return ONLINE_NEW(RenderPipeline);
			}
			static void Destroy(RenderPipeline* renderPipeline)
			{
				ONLINE_DELETE(renderPipeline);
			}
		};
		struct Lifecycle
		{
			friend class Online::Runtime::Module<RenderPipeline>;
		private:
			static bool Initialize(RenderPipeline* renderPipeline)
			{
				return renderPipeline->Initialize();
			}
			static void Release(RenderPipeline* renderPipeline)
			{
				renderPipeline->Release();
			}
		};

	private:
		struct RenderGraph
		{
			std::vector<RenderPass> Passes;
			std::vector<RenderItem> Items;
		};

	private:
		RenderPipeline() = default;
		~RenderPipeline() = default;

	public:
		RenderPipeline(const RenderPipeline&) = delete;
		RenderPipeline& operator=(const RenderPipeline&) = delete;
		RenderPipeline(RenderPipeline&&) = delete;
		RenderPipeline& operator=(RenderPipeline&&) = delete;

	private:
		bool Initialize();
		void Release();

	public:
		inline void NewPipeline() noexcept
		{
			graph.Passes.clear();
			graph.Items.clear();
		}
		inline void AddRenderPass(const Online::Core::CameraSnapshot& cameraSnapshot, 
			const Online::Render::RenderTargetSnapshot& renderTargetSnapshot, 
			const Online::Render::PostProcessSetting& postProcessSetting,
			Online::Render::RenderLayer cullingMask)
		{
			graph.Passes.emplace_back(cameraSnapshot, renderTargetSnapshot, postProcessSetting, cullingMask);
		}
		inline void AddRenderItem(
			Online::Core::StateFlags<Online::Render::RenderLayer> layerMask,
			Online::Asset::TextureID textureID,
			Online::Render::RenderQueue queue,
			const SDL_Rect& srcRect,
			const SDL_FRect& dstRect,
			uint8_t drawOrder = 0,
			uint8_t depth = 0,
			float rotation = 0.0f,
			const SDL_FPoint& pivot = { 0, 0 },
			SDL_RendererFlip flip = SDL_FLIP_NONE,
			Online::Core::Color color = Online::Core::Color::White
		)
		{
			graph.Items.emplace_back(
				layerMask.GetEnum(),
				textureID,
				queue,
				srcRect,
				dstRect,
				drawOrder,
				depth,
				rotation,
				pivot,
				flip,
				color
			);
		}
		inline void AddRenderItem(
			Online::Core::StateFlags<Online::Render::RenderLayer> layerMask,
			Online::Asset::TextureID textureID,
			Online::Render::RenderQueue queue,
			const SDL_Rect& srcRect,
			const SDL_FRect& dstRect,
			float rotation = 0.0f
		)
		{
			graph.Items.emplace_back(
				layerMask.GetEnum(),
				textureID,
				queue,
				srcRect,
				dstRect,
				0,
				0,
				rotation,
				SDL_FPoint{ 0,0 },
				SDL_FLIP_NONE,
				static_cast<const glm::vec4&>(Online::Core::Color::White)
			);
		}

	public:
		void Execute(Online::Render::Renderer*);

	private:
		void SubmitRenderGraph(Online::Render::Renderer*);

	private:
		RenderGraph graph;
	};
}