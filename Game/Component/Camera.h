#pragma once

#include<Core/Camera/CameraState.h>
#include<Core/StateFlags/StateFlags.h>
#include<Asset/Common/FuncTable.h>
#include<Input/Common/FuncTable.h>
#include<Render/Common/PostProcessSetting.h>
#include<Render/Common/RenderLayer.h>
#include<Core/Camera/CameraSnapshot.h>
#include<Render/Common/RenderTargetSnapshot.h>
#include<Game/Component/Component.h>

#include<entt/entt.hpp>

namespace Online::Game
{
    struct Camera : public Component
    {
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("cullingMask", static_cast<uint32_t>(cullingMask.GetEnum()));
            ctx.Write("renderTargetTex", static_cast<int>(renderTarget.TargetTexture));
            ctx.Write("renderOffsetX", renderTarget.Offset.x);
            ctx.Write("renderOffsetY", renderTarget.Offset.y);
            ctx.Write("renderSizeX", renderTarget.Size.x);
            ctx.Write("renderSizeY", renderTarget.Size.y);
            ctx.Write("cameraZoom", cameraState.Zoom);
            ctx.Write("cameraRotation", cameraState.Rotation);
            ctx.Write("postColorR", setting.Color.r);
            ctx.Write("postColorG", setting.Color.g);
            ctx.Write("postColorB", setting.Color.b);
            ctx.Write("postColorA", setting.Color.a);
            ctx.Write("postAlpha", setting.Alpha);
            ctx.Write("postBlend", setting.Blend);
            ctx.Write("postLinear", setting.Linear);
            ctx.Write("postRotation", setting.Rotation);
            ctx.Write("postFlip", setting.Flip);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            uint32_t mask = 0; ctx.Read("cullingMask", mask);
            cullingMask = static_cast<Render::RenderLayer>(mask);

            int texId = 0; ctx.Read("renderTargetTex", texId);
            renderTarget.TargetTexture = static_cast<Asset::TextureID>(texId);

            ctx.Read("renderOffsetX", renderTarget.Offset.x);
            ctx.Read("renderOffsetY", renderTarget.Offset.y);
            ctx.Read("renderSizeX", renderTarget.Size.x);
            ctx.Read("renderSizeY", renderTarget.Size.y);

            float zoom = 1.0f;
            float camRot = 0.0f;
            ctx.Read("cameraZoom", zoom);
            ctx.Read("cameraRotation", camRot);
            cameraState.Zoom = zoom;
            cameraState.Rotation = camRot;

            Online::Core::Color color = Online::Core::Color::White;
            uint8_t r = color.r, g = color.g, b = color.b, a = color.a;
            uint8_t alpha = 255;
            bool blend = true;
            bool linear = true;
            float postRot = 0.0f;
            bool flip = false;

            ctx.Read("postColorR", r);
            ctx.Read("postColorG", g);
            ctx.Read("postColorB", b);
            ctx.Read("postColorA", a);
            ctx.Read("postAlpha", alpha);
            ctx.Read("postBlend", blend);
            ctx.Read("postLinear", linear);
            ctx.Read("postRotation", postRot);
            ctx.Read("postFlip", flip);

            setting.Color.r = r; 
            setting.Color.g = g; 
            setting.Color.b = b; 
            setting.Color.a = a;
            setting.Alpha = alpha;
            setting.Blend = blend;
            setting.Linear = linear;
            setting.Rotation = postRot;
            setting.Flip = flip;
        }

    public:
        inline const Online::Core::CameraState& GetCameraState() const noexcept
        {
            return cameraState;
        }
        inline Online::Render::RenderLayer GetCullingMask() const noexcept
        {
            return cullingMask.GetEnum();
        }
        inline const Online::Render::PostProcessSetting& GetPostProcessSetting() const noexcept
        {
            return setting;
        }
        inline Online::Render::RenderTargetSnapshot GetRenderTarget() const noexcept
        {
            return renderTarget;
        }
        inline glm::ivec2 GetRenderOffset() const noexcept
        {
            return renderTarget.Offset;
        }
        inline glm::ivec2 GetRenderSize() const noexcept
        {
            return renderTarget.Size;
        }
        inline void SetCameraState(const Online::Core::CameraState& state) noexcept
        {
            cameraState = state;
        }
        inline void SetCullingMask(Online::Render::RenderLayer mask) noexcept
        {
            cullingMask = mask;
        }
        inline void SetRenderTarget(Online::Asset::TextureID id) noexcept
        {
            assert(id >= Online::Asset::TextureID::Tex_WindowBuffer);
            renderTarget.TargetTexture = id;
        }
        inline void SetRenderOffset(glm::ivec2 offset) noexcept
        {
            renderTarget.Offset = offset;
        }
        inline void SetRenderSize(glm::ivec2 size) noexcept
        {
            renderTarget.Size = size;
        }
        inline bool IsRenderTargetReady() const noexcept
        {
            return Online::Asset::IsTextureReady(renderTarget.TargetTexture);
        }
        inline void SetZone(float Zoom)
        {
			cameraState.Zoom = Zoom;
        }
        inline void SetIsWorld(bool flag)
        {
			cameraState.IsWorld = flag;
        }
        inline void SetClearColor(Core::Color color)
        {
            renderTarget.ClearColor = color;
        }
    public:
        Online::Core::CameraState cameraState;
        Online::Core::StateFlags<Online::Render::RenderLayer> cullingMask =
            static_cast<Online::Render::RenderLayer>(static_cast<uint32_t>(Online::Render::RenderLayer::Default));
        Online::Render::RenderTargetSnapshot renderTarget;
        Online::Render::PostProcessSetting setting;
    };
}