#pragma once

#include<Core/Allocate/Allocate.h>
#include<Core/Color/Color.h>
#include<Context/Common/Module.h>
#include<Context/Context.h>
#include<Client/Context/ClientContext.h>
#include<Render/Specific/RenderPass.h>
#include<Render/Specific/RenderItem.h>
#include<Render/Specific/RenderText.h>
#include<Asset/Common/FuncTable.h>

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

		inline void AddRenderText(const RenderText& text)
		{
			SDL_Texture* atlasTex = Asset::GetTexture(text.fontID);
			if (!atlasTex) return;

			const int cellSize = static_cast<int>(Asset::GetFontSize(text.fontID));
			const int cellW = cellSize;
			const int cellH = cellSize;
			if (cellW <= 0 || cellH <= 0) return;

			std::u32string u32text = Online::Core::Utf8ToUtf32(text.text);
			const float scale = text.scale;
			const float lineHeight = cellH * scale;
			const float spacing = text.LetterSpacing;

			int numLines = 1;
			float maxLineWidth = 0.0f;
			float currentLineWidth = 0.0f;

			for (char32_t ch : u32text)
			{
				if (ch == U'\n')
				{
					maxLineWidth = maxLineWidth > currentLineWidth ? maxLineWidth : currentLineWidth;
					currentLineWidth = 0.0f;
					numLines++;
					continue;
				}

				int advance = Asset::GetFontAtlasAdvance(text.fontID, ch);
				currentLineWidth += (advance + spacing) * scale;
			}
			maxLineWidth = maxLineWidth > currentLineWidth ? maxLineWidth : currentLineWidth;

			float totalWidth = maxLineWidth;
			float totalHeight = numLines * lineHeight;

			glm::vec2 anchorOffset{ 0.0f, 0.0f };
			switch (text.Anchor)
			{
				using enum Core::Anchor;
			case TopLeft:      anchorOffset = { 0.0f, 0.0f }; break;
			case TopRight:     anchorOffset = { totalWidth, 0.0f }; break;
			case BottomLeft:   anchorOffset = { 0.0f, totalHeight }; break;
			case BottomRight:  anchorOffset = { totalWidth, totalHeight }; break;
			case Center:       anchorOffset = { totalWidth * 0.5f, totalHeight * 0.5f }; break;
			default: break;
			}

			float startX = text.position.x - anchorOffset.x;
			float startY = text.position.y - anchorOffset.y;

			float cursorX = startX;
			float cursorY = startY;
			uint8_t baseDrawOrder = text.DrawOrder;

			for (char32_t ch : u32text)
			{
				if (ch == U'\n')
				{
					cursorX = startX;
					cursorY += lineHeight;
					continue;
				}

				SDL_Rect srcRect = Asset::GetFontAtlasSrcRect(text.fontID, ch);
				if (srcRect.w == 0)
				{
					srcRect = Asset::GetFontAtlasSrcRect(text.fontID, U'?');
					if (srcRect.w == 0) continue;
				}

				int advance = Asset::GetFontAtlasAdvance(text.fontID, ch);

				srcRect.w = advance;

				SDL_FRect dstRect;
				dstRect.x = cursorX;
				dstRect.y = cursorY;
				dstRect.w = advance * scale;
				dstRect.h = lineHeight;

				AddRenderItem(
					text.LayerMask,
					Asset::FontIDToTextureID(text.fontID),
					text.RenderQueueType,
					srcRect,
					dstRect,
					baseDrawOrder,
					text.Depth,
					text.Rotation,
					text.Pivot,
					SDL_FLIP_NONE,
					text.Color
				);

				cursorX += (advance + spacing) * scale;
			}
		}

		inline void AddRenderText(
			Online::Core::StateFlags<Online::Render::RenderLayer>  layerMask,
			Online::Asset::FontID                                  fontID,
			const std::string&                                     textStr,
			Online::Render::RenderQueue                            renderQueue,
			uint8_t                                                drawOrder,
			uint8_t                                                depth,
			const glm::vec2&                                       position,
			float                                                  scale,
			float                                                  rotation,
			const SDL_FPoint&                                      pivot,
			Online::Core::Color                                    color,
			Core::Anchor                                           anchor,
			float                                                  letterSpacing,
			float                                                  widthLimit = 0.0f
		)
		{
			SDL_Texture* atlasTex = Asset::GetTexture(fontID);
			if (!atlasTex) return;

			const int cellSize = static_cast<int>(Asset::GetFontSize(fontID));
			if (cellSize <= 0) return;

			const float lineHeight = static_cast<float>(cellSize) * scale;
			const float spacing = letterSpacing;

			std::u32string u32text = Online::Core::Utf8ToUtf32(textStr);

			int   numLines = 1;
			float maxLineWidth = 0.0f;
			float currentLineWidth = 0.0f;

			for (const char32_t ch : u32text)
			{
				if (ch == U'\n')
				{
					maxLineWidth = maxLineWidth > currentLineWidth ? maxLineWidth : currentLineWidth;
					currentLineWidth = 0.0f;
					++numLines;
					continue;
				}
				const int advance = Asset::GetFontAtlasAdvance(fontID, ch);
				currentLineWidth += (static_cast<float>(advance) + spacing) * scale;
			}
			maxLineWidth = maxLineWidth > currentLineWidth ? maxLineWidth : currentLineWidth;

			const float totalWidth = maxLineWidth;
			const float totalHeight = static_cast<float>(numLines) * lineHeight;

			glm::vec2 anchorOffset{ 0.0f, 0.0f };
			switch (anchor)
			{
				using enum Core::Anchor;
			case TopLeft:      anchorOffset = { 0.0f, 0.0f };               break;
			case TopRight:     anchorOffset = { totalWidth, 0.0f };         break;
			case BottomLeft:   anchorOffset = { 0.0f, totalHeight };        break;
			case BottomRight:  anchorOffset = { totalWidth, totalHeight };  break;
			case Center:       anchorOffset = { totalWidth * 0.5f, totalHeight * 0.5f }; break;
			default: break;
			}

			const float startX = position.x - anchorOffset.x;
			const float startY = position.y - anchorOffset.y;

			float cursorX = startX;
			float cursorY = startY;
			const Online::Asset::TextureID textureID = Asset::FontIDToTextureID(fontID);

			for (const char32_t ch : u32text)
			{
				if (ch == U'\n')
				{
					cursorX = startX;
					cursorY += lineHeight;
					continue;
				}

				SDL_Rect srcRect = Asset::GetFontAtlasSrcRect(fontID, ch);
				if (srcRect.w == 0)
				{
					srcRect = Asset::GetFontAtlasSrcRect(fontID, U'?');
					if (srcRect.w == 0) continue;
				}

				const int advance = Asset::GetFontAtlasAdvance(fontID, ch);
				srcRect.w = advance;

				SDL_FRect dstRect;
				dstRect.x = cursorX;
				dstRect.y = cursorY;
				dstRect.w = static_cast<float>(advance) * scale;
				dstRect.h = lineHeight;

				AddRenderItem(
					layerMask,
					textureID,
					renderQueue,
					srcRect,
					dstRect,
					drawOrder,
					depth,
					rotation,
					SDL_FPoint{ pivot.x, pivot.y },
					SDL_FLIP_NONE,
					color
				);

				cursorX += (static_cast<float>(advance) + spacing) * scale;
			}
		}

	public:
		void Execute(Online::Render::Renderer*);

	private:
		void SubmitRenderGraph(Online::Render::Renderer*);

	private:
		RenderGraph graph;
	};
}