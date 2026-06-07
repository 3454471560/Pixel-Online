#pragma once
#include <Core/Allocate/Allocate.h>
#include <Core/Thread/Thread.h>
#include <Render/Common/API.h>
#include <Render/Command/BeginPassCommand.h>
#include <Render/Command/DrawCommand.h>
#include <Render/Command/InstancedDrawCommand.h>
#include <Render/Command/LineDrawCommand.h>
#include <Render/Command/PostProcessCommand.h>
#include <Render/Specific/RenderPass.h>
#include <Render/Specific/RenderItem.h>
#include <Context/Common/Module.h>
#include <Thread/Common/FuncTable.h>
#include <Log/Common/FuncTable.h>

#include <vector>
#include <numeric>
#include <mutex>
#include <iostream>

namespace Online::Render
{
    class Renderer
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<Renderer>;
        private:
            static Renderer* Create(API api);
            static void Destroy(Renderer* r) { ONLINE_DELETE(r); }  
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<Renderer>;
        private:
            static bool Initialize(Renderer* renderer, void* nativeWindow)
            {
                return renderer->Initialize(nativeWindow);
            }
            static void Release(Renderer* r) { r->Release(); }
        };

    protected:
        struct RenderPassData
        {
            BeginPassCommand Config;
            std::vector<InstancedDrawCommand> Commands;
            std::vector<LineDrawCommand> LineCommands;
            PostProcessCommand PostConfig;
            
            void Clear() 
            { 
                Commands.clear();
				LineCommands.clear();
            }
        };
        struct RenderBuffer
        {
            std::vector<RenderPassData> Passes;
            uint32_t ActivePassCount = 0;

            void Clear()
            {
                for (uint32_t i = 0; i < ActivePassCount; ++i) {
                    Passes[i].Clear();
                }
                ActivePassCount = 0;
            }

            RenderPassData& AllocateNextPass()
            {
                if (static_cast<size_t>(ActivePassCount) >= Passes.size())
                {
                    Passes.emplace_back();
                }
                return Passes[ActivePassCount++];
            }
        };

    protected:
		class RenderPacket
		{
			friend class Renderer;
		private:
			RenderPacket() = default;
			~RenderPacket() = default;

		public:
			RenderPacket(const RenderPacket&) = delete;
			RenderPacket& operator=(const RenderPacket&) = delete;
			RenderPacket(RenderPacket&&) = delete;
			RenderPacket& operator=(RenderPacket&&) = delete;
        public:
            inline void ParseCommand(Online::Render::Renderer* renderer)
            {
                if (back.ActivePassCount == 0) return;

                for (uint32_t i = 0; i < back.ActivePassCount; ++i)
                {
                    const auto& renderPass = back.Passes[i];

                    renderer->ExecuteCommand(renderPass.Config);

                    for (const auto& drawCmd : renderPass.Commands)
                    {
                        renderer->ExecuteCommand(drawCmd);
                    }

                    for (const auto& lineCmd : renderPass.LineCommands)
                    {
                        renderer->ExecuteCommand(lineCmd);
                    }

					renderer->ExecuteCommand(renderPass.PostConfig);
                }

                back.Clear();
            }

		public:
            inline bool IsRemain() const noexcept
            {
                return back.ActivePassCount > 0;
            }
            inline void PushDrawCommandToPass(Online::Render::DrawCommand& cmd)
            {
                pass.emplace_back(cmd);
            }
            inline void PushLineCommandToPass(const LineDrawCommand& cmd)
            {
                linePass.emplace_back(cmd);
            }
            inline void CompilePass(const BeginPassCommand& passConfig,PostProcessCommand& processConfig)
            {
                if (pass.empty() && linePass.empty()) { return; }

                auto& currentPass = front.AllocateNextPass();
                currentPass.Config = passConfig;

                processConfig.SourceTexture = passConfig.RenderTarget.TargetTexture;
                processConfig.DrawOffset = passConfig.RenderTarget.Offset;
                processConfig.DrawSize = passConfig.RenderTarget.Size;
                currentPass.PostConfig = processConfig;

                const auto& cam = currentPass.Config.CameraSnapshot;
                const auto& rt = currentPass.Config.RenderTarget;

                visibleIndices.clear();
                size_t drawCommandCountSz = pass.size();
                assert(drawCommandCountSz <= UINT32_MAX && "Too many draw commands!");
                uint32_t drawCommandCount = static_cast<uint32_t>(drawCommandCountSz);
                visibleIndices.resize(drawCommandCount);
                std::iota(visibleIndices.begin(), visibleIndices.end(), 0);

                std::sort(visibleIndices.begin(), visibleIndices.end(),
                    [&](uint32_t a, uint32_t b) {
                        return pass[a].SortKey < pass[b].SortKey;
                    });

                currentPass.Commands.reserve(visibleIndices.size());
                for (uint32_t idx : visibleIndices)
                {
                    DrawCommand worldCmd = pass[idx];

                    glm::vec2 screenPos = cam.WorldToScreen({ worldCmd.DstRect.x, worldCmd.DstRect.y });
                    worldCmd.DstRect.x = screenPos.x;
                    worldCmd.DstRect.y = screenPos.y;

                    worldCmd.DstRect.w *= cam.Zoom;
                    worldCmd.DstRect.h *= cam.Zoom;

                    currentPass.Commands.emplace_back(worldCmd);
                }

                if (passConfig.CameraSnapshot.IsWorld)
                {
                    currentPass.LineCommands.reserve(linePass.size());
                    for (auto& lineCmd : linePass)
                    {
                        lineCmd.StartPoint = cam.WorldToScreen(lineCmd.StartPoint);
                        lineCmd.EndPoint = cam.WorldToScreen(lineCmd.EndPoint);

                        currentPass.LineCommands.emplace_back(lineCmd);
                    }

                    linePass.clear();
                }

                pass.clear();
            }
            inline void SwapBuffer()
            {
                std::swap(front, back);
            }
            inline void BeginFrame()
            {
                pass.clear();
                visibleIndices.clear();
                front.Clear();
            }

        private:
			inline RenderBuffer& GetFrontBuffer() noexcept 
            { 
                return front; 
            }
			inline const RenderBuffer& GetBackBuffer() const noexcept 
            {
                return back; 
            }

		private:
            RenderBuffer front;
            RenderBuffer back;
            std::vector<DrawCommand> pass;
            std::vector<LineDrawCommand> linePass;
            std::vector<uint32_t> visibleIndices;
		};
    public:
        Renderer() = default;
        virtual ~Renderer() = default;

    public:
        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

    protected:
        inline bool Initialize(void* nativeWindow)
        {
            isRunning.store(true, std::memory_order_release);
            renderThread = Online::Thread::RegisterThread("Render", &Online::Render::Renderer::BootstrapRenderThread, this, nativeWindow);
            if (renderThread == Online::Core::Thread::Identifier::Invalid) { throw std::runtime_error("Render Thread Register Fail!"); }
            return true;
        }
        inline void Release()
        {
            isRunning.store(false, std::memory_order_release);
            cond.notify_one();
            Online::Thread::UnregisterThread(renderThread);
        }

    protected:
        inline static void BootstrapRenderThread(void* renderer, void* nativeWindow)
        {
            static_cast<Online::Render::Renderer*>(renderer)->RenderThread(nativeWindow);
        }
        inline void RenderThread(void* nativeWindow)
        {
            if (!InitializeForRender(nativeWindow)) { throw std::runtime_error("Init Render Thread Fail!"); }


            while (true)
            {
                std::unique_lock<std::mutex> lock(mutex);
                cond.wait(lock, [this] { return renderPacket.IsRemain() ||
                    !isRunning.load(std::memory_order_acquire); });

                Online::Asset::SyncLoadedAssets();

                NewRenderFrame();
                ParseCommand();
                SwapBuffer();

                if (!renderPacket.IsRemain() && !isRunning.load(std::memory_order_acquire))
                {
                    break;
                }
            }

            ReleaseForRenderer();
        }
        inline bool InitializeForRender(void* nativeWindow)
        {
            return InitializeRenderContext(nativeWindow);
        }
        inline void ReleaseForRenderer()
        {
            ReleaseRenderContext();
        }
        virtual bool InitializeRenderContext(void* nativeWindow) = 0;
        virtual void ReleaseRenderContext() = 0;

    protected:
        inline void ParseCommand()
        {
            renderPacket.ParseCommand(this);
        }
        virtual void NewRenderFrame() = 0;
        virtual void SwapBuffer() const = 0;

    protected:
        virtual void ExecuteCommand(const BeginPassCommand& command) = 0;
        virtual void ExecuteCommand(const InstancedDrawCommand& command) const = 0;
        virtual void ExecuteCommand(const LineDrawCommand& command) const = 0;
        virtual void ExecuteCommand(const PostProcessCommand& command) const = 0;
    public:
        inline void BeginRenderFrame()
        {
            renderPacket.BeginFrame();
        }
        inline void BeginPass(const Online::Render::RenderPass& beginpasscommand)
        {
            currentPassConfig.CameraSnapshot = beginpasscommand.CameraSnapshot;
            currentPassConfig.RenderTarget = beginpasscommand.RenderTarget;
            currentProcessConfig = beginpasscommand.PostProcessSetting.BuildProcessCommand();
        }
        inline void Submit(const Online::Render::RenderItem& item)
        {
            Online::Render::DrawCommand cmd(item);

            renderPacket.PushDrawCommandToPass(cmd);
        }
        inline void SubmitLine(const glm::vec2& start, const glm::vec2& end, const Online::Core::Color& color, float thickness = 1.0f)
        {
            LineDrawCommand cmd(start, end, color, thickness);

            renderPacket.PushLineCommandToPass(cmd);
        }
        inline void EndPass()
        {
            renderPacket.CompilePass(currentPassConfig, currentProcessConfig);
        }
        inline void EndRenderFrame()
        {
            {
                std::lock_guard<std::mutex> lock(mutex);
                renderPacket.SwapBuffer();
            }

            cond.notify_one();
        }
    protected:
        std::mutex mutex;
        std::condition_variable cond;
        std::atomic<bool> isRunning = false;
        Online::Core::Thread::Identifier renderThread;
        RenderPacket renderPacket;
        BeginPassCommand currentPassConfig;
        PostProcessCommand currentProcessConfig;
    };
}