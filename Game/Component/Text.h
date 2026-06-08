#pragma once

#include <Game/Component/Component.h>
#include <Core/StateFlags/StateFlags.h>
#include <Core/Color/Color.h>
#include <Core/Anchor/Anchor.h>
#include <Render/Common/RenderLayer.h>
#include <Render/Common/RenderQueue.h>
#include <Asset/Common/ID/FontID.h>
#include <Asset/Common/FuncTable.h>
#include <Log/Common/FuncTable.h>

#include <glm.hpp>
#include <string>

inline std::string FromUtf8(const char8_t* str)
{
    if (str == nullptr) return {};
    return std::string(reinterpret_cast<const char*>(str));
}

namespace Online::Game
{
    struct Text : public Component
    {
    public:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("fontID", static_cast<int>(Font));
            ctx.Write("text", Text);
            ctx.Write("queue", static_cast<uint16_t>(RenderQueueType));
            ctx.Write("drawOrder", DrawOrder);
            ctx.Write("depth", Depth);
            ctx.Write("color", glm::vec4{ Color.r, Color.g, Color.b, Color.a });
            ctx.Write("offset", Offset);
            ctx.Write("rotation", Rotation);
            ctx.Write("pivotX", Pivot.x);
            ctx.Write("pivotY", Pivot.y);
            ctx.Write("anchor", static_cast<uint8_t>(Anchor));
            ctx.Write("visible", Visible);
            ctx.Write("widthLimit", WidthLimit);
            ctx.Write("letterSpacing", LetterSpacing);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            int fid = 0; ctx.Read("fontID", fid); Font = static_cast<Asset::FontID>(fid);
            ctx.Read("text", Text);


            uint16_t queue = 0; ctx.Read("queue", queue);
            RenderQueueType = static_cast<Render::RenderQueue>(queue);

            ctx.Read("drawOrder", DrawOrder);
            ctx.Read("depth", Depth);

            glm::vec4 col; ctx.Read("color", col);
            Color = { col.r, col.g, col.b, col.a };

            ctx.Read("offset", Offset);
            ctx.Read("rotation", Rotation);
            ctx.Read("pivotX", Pivot.x);
            ctx.Read("pivotY", Pivot.y);

            uint8_t anc = 0; ctx.Read("anchor", anc);
            Anchor = static_cast<Core::Anchor>(anc);

            ctx.Read("visible", Visible);
            ctx.Read("widthLimit", WidthLimit);
            ctx.Read("letterSpacing", LetterSpacing);
        }

    public:
        inline Asset::FontID GetFont() const noexcept 
        { 
            return Font; 
        }
        inline const std::string& GetText() const noexcept
        {
            return Text;
        }
        inline Render::RenderQueue GetRenderQueue() const noexcept 
        { 
            return RenderQueueType;
        }
        inline uint8_t GetDrawOrder() const noexcept 
        { 
            return DrawOrder;
        }
        inline uint8_t GetDepth() const noexcept
        {
            return Depth; 
        }
        inline const glm::vec4& GetColor() const noexcept 
        { 
            return Color;
        }
        inline const glm::vec2& GetOffset() const noexcept 
        { 
            return Offset;
        }
        inline float GetRotation() const noexcept
        { 
            return Rotation; 
        }
        inline SDL_FPoint GetPivot() const noexcept 
        { 
            return Pivot; 
        }
        inline Core::Anchor GetAnchor() const noexcept 
        { 
            return Anchor;
        }
        inline bool IsVisible() const noexcept 
        {
            return Visible; 
        }
        inline float GetWidthLimit() const noexcept 
        { 
            return WidthLimit;
        }
        inline float GetLetterSpacing() const noexcept 
        { 
            return LetterSpacing;
        }
        inline int GetFontHeight() const
        {
            TTF_Font* font = Online::Asset::GetFont(Font);
            return font ? TTF_FontHeight(font) : 0;
        }
       
        inline void SetFont(Asset::FontID id) noexcept 
        {
            Font = id; 
        }
        inline void SetText(const std::string& txt) noexcept 
        { 
            Text = txt;
        }
        inline void SetText(const char8_t* txt) noexcept
        {
            Text = reinterpret_cast<const char*>(txt);
        }
        inline void SetRenderQueue(Render::RenderQueue q) noexcept
        { 
            RenderQueueType = q;
        }
        inline void SetDrawOrder(uint8_t order) noexcept 
        { 
            DrawOrder = order;
        }
        inline void SetDepth(uint8_t d) noexcept
        { 
            Depth = d;
        }
        inline void SetColor(const glm::vec4& c) noexcept
        { 
            Color = c; 
        }
        inline void SetOffset(const glm::vec2& off) noexcept
        { 
            Offset = off; 
        }
        inline void SetRotation(float r) noexcept
        { 
            Rotation = r;
        }
        inline void SetPivot(const SDL_FPoint& p) noexcept
        { 
            Pivot = p;
        }
        inline void SetAnchor(Core::Anchor a) noexcept 
        { 
            Anchor = a; 
        }
        inline void SetVisible(bool v) noexcept 
        { 
            Visible = v;
        }
        inline void SetWidthLimit(float w) noexcept 
        { 
            WidthLimit = w; 
        }
        inline void SetLetterSpacing(float spacing) noexcept
        { 
            LetterSpacing = spacing;
        }
      
    private:
        Asset::FontID   Font = Asset::FontID::Ipix;
        std::string     Text;
        Render::RenderQueue RenderQueueType = Render::RenderQueue::World;
        uint8_t         DrawOrder = 0;
        uint8_t         Depth = 0;
        glm::vec4       Color = { 1,1,1,1 };
        glm::vec2       Offset = { 0,0 };
        float           Rotation = 0.0f;
        SDL_FPoint      Pivot = { 0,0 };
        Core::Anchor    Anchor = Core::Anchor::Center;
        bool            Visible = true;
        float           WidthLimit = 0.0f;
        float           LetterSpacing = 0.0f;
    };
}