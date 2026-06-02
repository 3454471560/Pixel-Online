#pragma once
#include <Render/Frontend/Renderer.h>
#include <SDL.h>

namespace Online::Render
{
    class SDLRenderer : public Online::Render::Renderer
    {
        friend class Online::Render::Renderer;
    private:
        SDLRenderer() = default;
        ~SDLRenderer() override = default;

    public:
        SDLRenderer(const SDLRenderer&) = delete;
        SDLRenderer& operator=(const SDLRenderer&) = delete;
    protected:
        bool InitializeRenderContext(void* nativeWindow) override;
        void ReleaseRenderContext() override;

        void NewRenderFrame() override;
        void SwapBuffer() const override;

        void ExecuteCommand(const Online::Render::BeginPassCommand& command) override;
        void ExecuteCommand(const Online::Render::InstancedDrawCommand& command) const override;
        void ExecuteCommand(const Online::Render::LineDrawCommand& command) const override;
        void ExecuteCommand(const Online::Render::PostProcessCommand& command) const override;

    private:
        SDL_Window* sdlWindow = nullptr;
        SDL_Renderer* sdlRenderer = nullptr;
        SDL_Texture* screenTarget = nullptr;
    };
}