#include <Render/Backend/SDL/SDLRenderer.h>
#include <Asset/Common/FuncTable.h>

#include <stdexcept>
#include <algorithm>

bool Online::Render::SDLRenderer::InitializeRenderContext(void* nativeWindow)
{
    sdlWindow = reinterpret_cast<SDL_Window*>(nativeWindow);
    if (!sdlWindow)
    {
        return false;
    }

    sdlRenderer = SDL_CreateRenderer(
        sdlWindow,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE
    );

    if (!sdlRenderer)
    {
        return false;
    }

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    Online::Asset::SetRenderer(sdlRenderer);
    Online::Asset::InitOffScreen();
    
    return true;
}

void Online::Render::SDLRenderer::ReleaseRenderContext()
{
    if (screenTarget)
    {
        SDL_DestroyTexture(screenTarget);
        screenTarget = nullptr;
    }

    if (sdlRenderer)
    {
        SDL_DestroyRenderer(sdlRenderer);
        sdlRenderer = nullptr;
    }

    sdlWindow = nullptr;
}

void Online::Render::SDLRenderer::NewRenderFrame()
{
    SDL_SetRenderTarget(sdlRenderer, nullptr);

    int w, h;
    SDL_GetWindowSize(sdlWindow, &w, &h);
    SDL_RenderSetViewport(sdlRenderer, nullptr);
    SDL_RenderSetClipRect(sdlRenderer, nullptr);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
}

void Online::Render::SDLRenderer::SwapBuffer() const
{
    SDL_RenderPresent(sdlRenderer);
}

void Online::Render::SDLRenderer::ExecuteCommand(const Online::Render::BeginPassCommand& command)
{
    SDL_Rect vp;
    if (command.RenderTarget.TargetTexture == Online::Asset::TextureID::Tex_WindowBuffer)
    {
        SDL_SetRenderTarget(sdlRenderer, nullptr);
        vp = {
            command.RenderTarget.Offset.x,
            command.RenderTarget.Offset.y,
            command.RenderTarget.Size.x,
            command.RenderTarget.Size.y
        };
    }
    else
    {
        SDL_Texture* targetTexture = Online::Asset::GetTexture(command.RenderTarget.TargetTexture);
        SDL_SetRenderTarget(sdlRenderer, targetTexture);
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);
        SDL_RenderClear(sdlRenderer);
        vp = { 0,0,(int)command.RenderTarget.Size.x,(int)command.RenderTarget.Size.y };
    }
 
    SDL_RenderSetViewport(sdlRenderer, &vp);

    if (command.RenderTarget.ClearEnable)
    {
        SDL_SetRenderDrawColor(
            sdlRenderer,
            static_cast<uint8_t>(command.RenderTarget.ClearColor.r * 255),
            static_cast<uint8_t>(command.RenderTarget.ClearColor.g * 255),
            static_cast<uint8_t>(command.RenderTarget.ClearColor.b * 255),
            static_cast<uint8_t>(command.RenderTarget.ClearColor.a * 255)
        );

        SDL_Rect clearRect{ 0, 0, vp.w, vp.h };
        SDL_RenderFillRect(sdlRenderer, &clearRect);
    }
}

void Online::Render::SDLRenderer::ExecuteCommand(const Online::Render::InstancedDrawCommand& command) const
{
    if (!command.Texture) { return; }

    SDL_SetTextureColorMod(
        command.Texture,
        static_cast<uint8_t>(command.Color.r * 255),
        static_cast<uint8_t>(command.Color.g * 255),
        static_cast<uint8_t>(command.Color.b * 255)
    );

    SDL_SetTextureAlphaMod(
        command.Texture,
        static_cast<uint8_t>(command.Color.a * 255)
    );

    SDL_SetTextureBlendMode(command.Texture, SDL_BLENDMODE_BLEND);

    SDL_RenderCopyExF(
        sdlRenderer,
        command.Texture,
        &command.SrcRect,
        &command.DstRect,
        glm::degrees(command.Rotation),
        &command.Pivot,
        command.Flip
    );
}

void Online::Render::SDLRenderer::ExecuteCommand(const Online::Render::LineDrawCommand& command) const
{
    if (!sdlRenderer) { return; }

    SDL_SetRenderDrawColor(
        sdlRenderer,
        static_cast<uint8_t>(command.Color.r * 255),
        static_cast<uint8_t>(command.Color.g * 255),
        static_cast<uint8_t>(command.Color.b * 255),
        static_cast<uint8_t>(command.Color.a * 255)
    );

    if (command.Thickness <= 1.0f)
    {
        SDL_RenderDrawLineF(
            sdlRenderer,
            command.StartPoint.x, command.StartPoint.y,
            command.EndPoint.x, command.EndPoint.y
        );
    }
    else
    {
        glm::vec2 lineDir = command.EndPoint - command.StartPoint;
        glm::vec2 lineNormal = glm::normalize(glm::vec2(-lineDir.y, lineDir.x));
        float halfThickness = command.Thickness * 0.5f;

        glm::vec2 p1 = command.StartPoint - lineNormal * halfThickness;
        glm::vec2 p2 = command.StartPoint + lineNormal * halfThickness;
        glm::vec2 p3 = command.EndPoint + lineNormal * halfThickness;
        glm::vec2 p4 = command.EndPoint - lineNormal * halfThickness;

        SDL_Vertex vertices[] = {
            {{p1.x, p1.y}, {255, 255, 255, 255}, {0, 0}},
            {{p2.x, p2.y}, {255, 255, 255, 255}, {0, 0}},
            {{p3.x, p3.y}, {255, 255, 255, 255}, {0, 0}},
            {{p4.x, p4.y}, {255, 255, 255, 255}, {0, 0}}
        };

        SDL_RenderGeometry(
            sdlRenderer,
            nullptr,
            vertices,
            4,
            nullptr,
            0
        );
    }
}

void Online::Render::SDLRenderer::ExecuteCommand(const Online::Render::PostProcessCommand& command) const
{
    SDL_SetRenderTarget(sdlRenderer, nullptr);
    SDL_RenderSetViewport(sdlRenderer, nullptr);
    SDL_RenderSetClipRect(sdlRenderer, nullptr);

    if (command.SourceTexture == Online::Asset::TextureID::Tex_WindowBuffer) { return; }

	SDL_Texture* texture = Online::Asset::GetTexture(command.SourceTexture);
    SDL_SetTextureAlphaMod(texture, command.Alpha);
    SDL_SetTextureBlendMode(texture, command.BlendMode);
    SDL_SetTextureScaleMode(texture, command.ScaleMode);
    SDL_SetTextureColorMod(texture, command.Color.r * 255, command.Color.g * 255, command.Color.b * 255);
    SDL_Rect srcRect{
       0,
       0,
       (command.DrawSize.x),
       (command.DrawSize.y)};
    SDL_FRect dstRect{
        static_cast<float>(command.DrawOffset.x),
        static_cast<float>(command.DrawOffset.y),
        static_cast<float>(command.DrawSize.x),
        static_cast<float>(command.DrawSize.y)}; 
    SDL_RenderCopyExF(sdlRenderer, texture, &srcRect, &dstRect, 0, nullptr, SDL_FLIP_NONE);
}