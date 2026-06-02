#pragma once

#include<Core/StateFlags/StateFlags.h>
#include<Core/Color/Color.h>
#include<Core/Anchor/Anchor.h>
#include<Render/Common/RenderLayer.h>
#include<Render/Common/RenderQueue.h>
#include<Asset/Common/ID/TextureID.h>
#include<Asset/Common/FuncTable.h>
#include<Game/Component/Component.h>
#include<Game/Common/Direction.h>
#include<Game/Common/SrcRectMode.h>
#include<Log/Common/FuncTable.h>

#include<concepts>
#include<glm.hpp>
#include<SDL.h>
#include<algorithm>
#include<cmath>

namespace Online::Game
{
    struct Sprite : public Component
    {
    public:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("texID", static_cast<int>(TextureID));
            ctx.Write("layer", static_cast<uint32_t>(LayerMask.GetEnum()));
            ctx.Write("queue", static_cast<uint16_t>(RenderQueueType));
            ctx.Write("drawOrder", DrawOrder);
            ctx.Write("depth", Depth);
            ctx.Write("color", glm::vec4{ Color.r, Color.g, Color.b, Color.a });
            ctx.Write("renderOffset", RenderOffset);
            ctx.Write("flip", static_cast<int>(Flip));
            ctx.Write("pivotX", Pivot.x);
            ctx.Write("pivotY", Pivot.y);
            ctx.Write("gridCols", GridCols);
            ctx.Write("gridRows", GridRows);
            ctx.Write("frame", FrameIndex);
            ctx.Write("visible", Visible);
            ctx.Write("srcRectMode", static_cast<uint8_t>(SrcRectMode));
            if (SrcRectMode == SrcRectMode::Progress)
            {
                ctx.Write("progress", Progress);
                ctx.Write("progressDir", static_cast<uint8_t>(ProgressDir));
            }
            ctx.Write("useOverrideSize", UseOverrideSize);
            if (UseOverrideSize)
                ctx.Write("baseSize", BaseSize);
            ctx.Write("anchor", static_cast<uint8_t>(RenderAnchor));  // 新增
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            int tex = 0;
            ctx.Read("texID", tex);
            TextureID = static_cast<Asset::TextureID>(tex);

            uint32_t layer = 0;
            ctx.Read("layer", layer);
            LayerMask = static_cast<Render::RenderLayer>(layer);

            uint16_t queue = 0;
            ctx.Read("queue", queue);
            RenderQueueType = static_cast<Render::RenderQueue>(queue);

            ctx.Read("drawOrder", DrawOrder);
            ctx.Read("depth", Depth);

            glm::vec4 col;
            ctx.Read("color", col);
            Color = { col.r, col.g, col.b, col.a };

            ctx.Read("renderOffset", RenderOffset);

            int flipVal = 0;
            ctx.Read("flip", flipVal);
            Flip = static_cast<SDL_RendererFlip>(flipVal);

            ctx.Read("pivotX", Pivot.x);
            ctx.Read("pivotY", Pivot.y);

            ctx.Read("gridCols", GridCols);
            ctx.Read("gridRows", GridRows);

            ctx.Read("frame", FrameIndex);

            ctx.Read("visible", Visible);

            uint8_t srcMode = 0;
            ctx.Read("srcRectMode", srcMode);
            SrcRectMode = static_cast<Game::SrcRectMode>(srcMode);

            if (SrcRectMode == SrcRectMode::Progress)
            {
                float prog = 1.0f;
                ctx.Read("progress", prog);
                Progress = prog;
                uint8_t dir = 0;
                ctx.Read("progressDir", dir);
                ProgressDir = static_cast<ProgressDirection>(dir);
            }

            bool useOverride = false;
            ctx.Read("useOverrideSize", useOverride);
            UseOverrideSize = useOverride;
            if (useOverride)
            {
                glm::vec2 base;
                ctx.Read("baseSize", base);
                BaseSize = base;
            }

            // 新增锚点读取，默认为中心（兼容旧数据缺失字段）
            uint8_t anchorVal = static_cast<uint8_t>(Core::Anchor::Center);
            ctx.Read("anchor", anchorVal);
            RenderAnchor = static_cast<Core::Anchor>(anchorVal);

            SetTexture(TextureID); // 触发 UpdateSrcRect，初始化 ProgressFrameOffset
        }

    public:
        inline Asset::TextureID GetTexture() const noexcept { return TextureID; }
        inline Core::StateFlags<Render::RenderLayer> GetLayerMask() const noexcept { return LayerMask; }
        inline Render::RenderQueue GetRenderQueue() const noexcept { return RenderQueueType; }
        inline uint8_t GetDrawOrder() const noexcept { return DrawOrder; }
        inline uint8_t GetDepth() const noexcept { return Depth; }
        inline const SDL_Rect GetSrcRect() const noexcept { return SrcRect; }
        inline glm::vec2 GetAnchorRatio() const noexcept
        {
            switch (RenderAnchor)
            {
            case Core::Anchor::TopLeft:     return { 0.0f, 0.0f };
            case Core::Anchor::TopRight:    return { 1.0f, 0.0f };
            case Core::Anchor::BottomLeft:  return { 0.0f, 1.0f };
            case Core::Anchor::BottomRight: return { 1.0f, 1.0f };
            case Core::Anchor::Center:
            default:                  return { 0.5f, 0.5f };
            }
        }
        inline const SDL_FRect GetDstRect(glm::vec2 worldPos, glm::vec2 worldScale) const
        {
            // 可见尺寸（受进度影响）
            glm::vec2 visibleSize = GetSize() * worldScale;
            // 完整帧尺寸（不受进度影响）
            glm::vec2 fullSize = BaseSize * worldScale;

            // 锚点在完整帧中的偏移量
            glm::vec2 anchorRatio = GetAnchorRatio();
            glm::vec2 anchorOffsetFull = fullSize * anchorRatio;

            // 进度模式下的帧内偏移（世界单位）
            glm::vec2 progressWorldOffset = (SrcRectMode == SrcRectMode::Progress)
                ? ProgressFrameOffset * worldScale
                : glm::vec2(0.0f);

            // 左上角 = 世界坐标 + 渲染偏移 - 锚点偏移 + 进度帧内偏移
            float dstX = worldPos.x + GetRenderOffset().x - anchorOffsetFull.x + progressWorldOffset.x;
            float dstY = worldPos.y + GetRenderOffset().y - anchorOffsetFull.y + progressWorldOffset.y;

            return SDL_FRect{ dstX, dstY, visibleSize.x, visibleSize.y };
        }
        inline SDL_RendererFlip GetFlip() const noexcept { return Flip; }
        inline const SDL_FPoint& GetPivot() const noexcept { return Pivot; }
        inline glm::vec2 GetSize() const noexcept { return OverrideSize; }
        inline uint8_t GetGridCols() const noexcept { return GridCols; }
        inline uint8_t GetGridRows() const noexcept { return GridRows; }
        inline uint8_t GetFrame() const noexcept { return FrameIndex; }
        inline bool IsUsingAutoSrcRect() const noexcept { return SrcRectMode == SrcRectMode::AutoGrid; }
        inline const glm::vec4& GetColor() const noexcept { return Color; }
        inline float GetAlpha() const noexcept { return Color.a; }
        inline bool IsVisible() const noexcept { return Visible; }
        inline const glm::vec2& GetRenderOffset() const noexcept { return RenderOffset; }
        inline bool IsFlippedX() const noexcept { return (Flip & SDL_FLIP_HORIZONTAL) != 0; }
        inline bool IsFlippedY() const noexcept { return (Flip & SDL_FLIP_VERTICAL) != 0; }
        inline float GetProgress() const noexcept { return Progress; }
        inline ProgressDirection GetProgressDirection() const noexcept { return ProgressDir; }
        inline bool IsInProgressMode() const noexcept { return SrcRectMode == SrcRectMode::Progress; }
        inline glm::vec2 GetTextureSize() const noexcept { return TextureSize; }
        inline void SetAnchor(Core::Anchor anchor) noexcept { RenderAnchor = anchor; }
        inline Core::Anchor GetAnchor() const noexcept { return RenderAnchor; }
        inline void UseTextureSize() noexcept
        {
            UseOverrideSize = false;
            UpdateBaseSizeFromTexture();
            if (SrcRectMode != SrcRectMode::Progress)
                OverrideSize = BaseSize;
        }
        inline void SetTexture(Asset::TextureID texID) noexcept
        {
            TextureID = texID;
            TextureSize = Asset::GetTextureSize(texID);
            UpdateBaseSizeFromTexture();
            UpdateSrcRect();
            if (SrcRectMode != SrcRectMode::Progress && !UseOverrideSize)
                OverrideSize = BaseSize;
        }
        inline void SetLayer(Render::RenderLayer layer) noexcept { LayerMask = layer; }
        inline void SetRenderQueue(Render::RenderQueue queue) noexcept { RenderQueueType = queue; }
        inline void SetDrawOrder(uint8_t order) noexcept { DrawOrder = order; }
        inline void SetDepth(uint8_t depth) noexcept { Depth = depth; }
        inline void SetSrcRect(const SDL_Rect& rect) noexcept
        {
            SrcRect = rect;
            SrcRectMode = SrcRectMode::Manual;
            LastProgressPixel = -1;
        }
        inline void SetPivot(const glm::vec2& pivot) noexcept { Pivot = { pivot.x, pivot.y }; }
        inline void SetSize(const glm::vec2& size) noexcept
        {
            BaseSize = size;
            UseOverrideSize = true;
            if (SrcRectMode != SrcRectMode::Progress)
                OverrideSize = BaseSize;
        }
        inline void SetGrid(uint8_t cols, uint8_t rows) noexcept
        {
            GridCols = cols > 1 ? cols : 1;
            GridRows = rows > 1 ? rows : 1;
            SrcRectMode = SrcRectMode::AutoGrid;
            UpdateBaseSizeFromTexture();
            UpdateSrcRect();
            if (SrcRectMode != SrcRectMode::Progress && !UseOverrideSize)
                OverrideSize = BaseSize;
        }
        template<std::same_as<uint8_t> T>
        inline void SetFrame(T index) noexcept
        {
            FrameIndex = index;
            UpdateSrcRect();
        }
        inline void SetColor(const glm::vec4& color) noexcept { Color = color; }
        inline void SetColor(float r, float g, float b, float a = 1.0f) noexcept { Color = { r, g, b, a }; }
        inline void SetAlpha(float alpha) noexcept { Color.a = alpha; }
        inline void OnDisable() noexcept override { Visible = false; }
        inline void OnEnable() noexcept override { Visible = true; }
        inline void SetRenderOffset(const glm::vec2& offset) noexcept { RenderOffset = offset; }
        inline void SetRenderOffset(float x, float y) noexcept { RenderOffset = { x, y }; }
        inline void SetFlip(SDL_RendererFlip flip) noexcept { Flip = flip; }
        inline void SetFlipX(bool flip) noexcept
        {
            Flip = flip ? (SDL_RendererFlip)(Flip | SDL_FLIP_HORIZONTAL) : (SDL_RendererFlip)(Flip & ~SDL_FLIP_HORIZONTAL);
        }
        inline void SetFlipY(bool flip) noexcept
        {
            Flip = flip ? (SDL_RendererFlip)(Flip | SDL_FLIP_VERTICAL) : (SDL_RendererFlip)(Flip & ~SDL_FLIP_VERTICAL);
        }
        inline void SetProgress(float progress) noexcept
        {
            Progress = std::clamp(progress, 0.0f, 1.0f);
            SrcRectMode = SrcRectMode::Progress;
            LastProgressPixel = -1;
            UpdateSrcRect();
        }
        inline void SetProgressDirection(ProgressDirection direction) noexcept
        {
            ProgressDir = direction;
            if (SrcRectMode == SrcRectMode::Progress)
            {
                LastProgressPixel = -1;
                UpdateSrcRect();
            }
        }
        inline void DisableProgressMode() noexcept
        {
            if (SrcRectMode == SrcRectMode::Progress)
            {
                OverrideSize = BaseSize;
                SrcRectMode = (GridCols > 1 || GridRows > 1) ? SrcRectMode::AutoGrid : SrcRectMode::Manual;
                UpdateSrcRect();
            }
        }
    private:
        inline void UpdateSrcRect() noexcept
        {
            switch (SrcRectMode)
            {
            case SrcRectMode::AutoGrid:
                ProgressFrameOffset = glm::vec2(0.0f); // 清空进度偏移
                UpdateAutoGridSrcRect();
                break;
            case SrcRectMode::Progress:
                UpdateProgressSrcRect();
                break;
            case SrcRectMode::Manual:
                ProgressFrameOffset = glm::vec2(0.0f);
                break;
            }
        }
        inline void UpdateAutoGridSrcRect() noexcept
        {
            glm::ivec2 texSize = Online::Asset::GetTextureSize(TextureID);
            if (texSize.x <= 0 || texSize.y <= 0) return;

            int frameW = texSize.x / GridCols;
            int frameH = texSize.y / GridRows;
            if (frameW <= 0 || frameH <= 0) return;

            int totalFrames = GridCols * GridRows;
            uint8_t safeIndex = FrameIndex;
            if (safeIndex >= totalFrames) safeIndex = static_cast<uint8_t>(totalFrames - 1);

            int col = safeIndex % GridCols;
            int row = safeIndex / GridCols;

            SrcRect.x = col * frameW;
            SrcRect.y = row * frameH;
            SrcRect.w = frameW;
            SrcRect.h = frameH;
        }
        inline void UpdateProgressSrcRect() noexcept
        {
            if (TextureSize.x <= 0 || TextureSize.y <= 0) return;

            int fullFrameTexW = TextureSize.x / GridCols;
            int fullFrameTexH = TextureSize.y / GridRows;
            if (fullFrameTexW <= 0 || fullFrameTexH <= 0) return;

            int totalFrames = GridCols * GridRows;
            uint8_t safeIndex = FrameIndex;
            if (safeIndex >= totalFrames) safeIndex = static_cast<uint8_t>(totalFrames - 1);
            int frameCol = safeIndex % GridCols;
            int frameRow = safeIndex / GridCols;
            int frameBaseX = frameCol * fullFrameTexW;
            int frameBaseY = frameRow * fullFrameTexH;

            int maxPixelWidth = 0;
            bool horizontal = false;
            switch (ProgressDir)
            {
            case ProgressDirection::LeftToRight:
            case ProgressDirection::RightToLeft:
                maxPixelWidth = fullFrameTexW; horizontal = true; break;
            case ProgressDirection::TopToBottom:
            case ProgressDirection::BottomToTop:
                maxPixelWidth = fullFrameTexH; horizontal = false; break;
            }

            int targetPixelWidth = static_cast<int>(std::round(Progress * maxPixelWidth));
            targetPixelWidth = std::clamp(targetPixelWidth, 0, maxPixelWidth);
            if (targetPixelWidth == LastProgressPixel) return;
            LastProgressPixel = targetPixelWidth;

            float ratio = (maxPixelWidth > 0) ? (static_cast<float>(targetPixelWidth) / maxPixelWidth) : 0.0f;

            switch (ProgressDir)
            {
            case ProgressDirection::LeftToRight:
                SrcRect.x = frameBaseX;
                SrcRect.y = frameBaseY;
                SrcRect.w = targetPixelWidth;
                SrcRect.h = fullFrameTexH;
                OverrideSize.x = BaseSize.x * ratio;
                OverrideSize.y = BaseSize.y;
                ProgressFrameOffset = glm::vec2(0.0f, 0.0f);
                break;

            case ProgressDirection::RightToLeft:
            {
                int offsetX = fullFrameTexW - targetPixelWidth;
                SrcRect.x = frameBaseX + offsetX;
                SrcRect.y = frameBaseY;
                SrcRect.w = targetPixelWidth;
                SrcRect.h = fullFrameTexH;
                OverrideSize.x = BaseSize.x * ratio;
                OverrideSize.y = BaseSize.y;
                ProgressFrameOffset = glm::vec2(offsetX, 0.0f);
                break;
            }
            case ProgressDirection::TopToBottom:
                SrcRect.x = frameBaseX;
                SrcRect.y = frameBaseY;
                SrcRect.w = fullFrameTexW;
                SrcRect.h = targetPixelWidth;
                OverrideSize.x = BaseSize.x;
                OverrideSize.y = BaseSize.y * ratio;
                ProgressFrameOffset = glm::vec2(0.0f, 0.0f);
                break;

            case ProgressDirection::BottomToTop:
            {
                int offsetY = fullFrameTexH - targetPixelWidth;
                SrcRect.x = frameBaseX;
                SrcRect.y = frameBaseY + offsetY;
                SrcRect.w = fullFrameTexW;
                SrcRect.h = targetPixelWidth;
                OverrideSize.x = BaseSize.x;
                OverrideSize.y = BaseSize.y * ratio;
                ProgressFrameOffset = glm::vec2(0.0f, offsetY);
                break;
            }
            }
        }
        inline void UpdateBaseSizeFromTexture() noexcept
        {
            glm::ivec2 texSize = Online::Asset::GetTextureSize(TextureID);
            if (texSize.x <= 0 || texSize.y <= 0)
            {
                Online::Log::Error("Failed to get size " + Online::Asset::TextureIDToString(TextureID));
                return;
            }
            int frameW = texSize.x / GridCols;
            int frameH = texSize.y / GridRows;
            BaseSize = { (float)frameW, (float)frameH };
        }
    private:
        Asset::TextureID TextureID = Asset::TextureID::Tex_Default;
        Core::StateFlags<Render::RenderLayer> LayerMask = Render::RenderLayer::Default;
        Render::RenderQueue RenderQueueType = Render::RenderQueue::World;
        uint8_t DrawOrder = 0;
        uint8_t Depth = 0;
        SDL_Rect SrcRect = { 0, 0, 0, 0 };
        SDL_FPoint Pivot = { 0.5f, 0.5f };
        SDL_RendererFlip Flip = SDL_FLIP_NONE;
        glm::vec2 OverrideSize = { 0.0f, 0.0f };
        bool UseOverrideSize = false;
        uint8_t GridCols = 1;
        uint8_t GridRows = 1;
        uint8_t FrameIndex = 0;
        Online::Core::Color Color = Online::Core::Color::White;
        glm::vec2 RenderOffset = { 0.0f, 0.0f };
        bool Visible = true;
        SrcRectMode SrcRectMode = SrcRectMode::Manual;
        float Progress = 1.0f;
        ProgressDirection ProgressDir = ProgressDirection::LeftToRight;
        glm::vec2 BaseSize = { 0.0f, 0.0f };
        glm::vec2 TextureSize = { 0.0f, 0.0f };
        int LastProgressPixel = -1;

        Core::Anchor RenderAnchor = Core::Anchor::Center;
        glm::vec2 ProgressFrameOffset = { 0.0f, 0.0f };
    };
}