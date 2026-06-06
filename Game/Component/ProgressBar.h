#pragma once

#include <Game/Component/Sprite.h>
#include <Game/Component/Component.h>
#include <Game/Common/Direction.h>
#include <Game/Common/FuncTable.h>
#include <Input/Common/FuncTable.h>

#include <entt/entt.hpp>
#include <functional>
#include <glm.hpp>
#include <cstdint>

namespace Online::Game
{
    struct ProgressBar : public Component
    {
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("targetProgress", TargetProgress);
            ctx.Write("currentProgress", CurrentProgress);
            ctx.Write("smoothTime", SmoothTime);
            ctx.Write("smoothPaused", SmoothPaused);
            ctx.Write("direction", static_cast<uint8_t>(ProgressDirection));
            ctx.Write("autoSyncDir", AutoSyncDirection);
            ctx.Write("forceFgTop", ForceForegroundOnTop);
            ctx.Write("maxSpeed", MaxSpeed);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("targetProgress", TargetProgress);
            ctx.Read("currentProgress", CurrentProgress);
            ctx.Read("smoothTime", SmoothTime);
            ctx.Read("smoothPaused", SmoothPaused);
            uint8_t dir = 0; ctx.Read("direction", dir);
            ProgressDirection = static_cast<Game::ProgressDirection>(dir);
            ctx.Read("autoSyncDir", AutoSyncDirection);
            ctx.Read("forceFgTop", ForceForegroundOnTop);
            ctx.Read("maxSpeed", MaxSpeed);
        }

    public:
        using ProgressCompleteCallback = std::function<void()>;
        using ProgressChangedCallback = std::function<void(float, float)>;

        inline void SetBackgroundEntity(entt::entity entity) noexcept
        {
            BackgroundSprite = Online::Game::GetRegistry().try_get<Sprite>(entity);
            NeedSyncRenderOrder = true;
        }

        inline void SetBackgroundSprite(Sprite* sprite) noexcept
        {
			BackgroundSprite = sprite;
            NeedSyncRenderOrder = true;
        }

        inline void SetForegroundEntity(entt::entity entity) noexcept
        {
            ForegroundSprite = Online::Game::GetRegistry().try_get<Sprite>(entity);
            NeedSyncRenderOrder = true;
        }

        inline void SetForegroundSprite(Sprite* sprite) noexcept
        {
            ForegroundSprite = sprite;
            NeedSyncRenderOrder = true;
        }

        inline void SetNeedSyncRenderOrder(bool flag)
        {
            NeedSyncRenderOrder = flag;
        }

        inline void SetNeedSyncDirection(bool flag)
        {
            NeedSyncDirection = flag;
        }

        inline Sprite* GetBackgroundEntity() const noexcept
        {
            return BackgroundSprite;
        }

        inline Sprite* GetForegroundEntity() const noexcept
        {
            return ForegroundSprite;
        }

        inline bool GetNeedSyncRenderOrder() const 
        {
            return NeedSyncRenderOrder;
        }

        inline bool GetNeedSyncDirection() const
        {
            return NeedSyncDirection;
        }

        inline void SetForceForegroundOnTop(bool enable) noexcept
        {
            ForceForegroundOnTop = enable;
            if (enable)
                NeedSyncRenderOrder = true;
        }

        inline bool IsForceForegroundOnTop() const noexcept
        {
            return ForceForegroundOnTop;
        }

        inline float GetTargetProgress() const noexcept 
        { 
            return TargetProgress; 
        }

        inline float GetCurrentProgress() const noexcept 
        { 
            return CurrentProgress; 
        }

        inline ProgressChangedCallback& GetProgressChangeCallback()
        {
            return OnProgressChanged;
        }

        inline void SetProgress(float progress) noexcept
        {
            TargetProgress = glm::clamp(progress, 0.0f, 1.0f);
            if (TargetProgress < 1.0f) HasTriggeredComplete = false;
            if(ForegroundSprite && progress == 0)
                ForegroundSprite->SetProgress(progress);
        }

        inline void SetCurrentProgress(float progress) noexcept
        {
            CurrentProgress = progress;
        }

        inline void SetProgressImmediate(float progress) noexcept
        {
            TargetProgress = glm::clamp(progress, 0.0f, 1.0f);
            CurrentProgress = TargetProgress;
            HasTriggeredComplete = false;
            if (OnProgressChanged) OnProgressChanged(CurrentProgress, TargetProgress);
        }

        inline float GetSmoothTime() const noexcept
        { 
            return SmoothTime;
        }

        inline void SetSmoothTime(float time) noexcept 
        { 
            SmoothTime = (time > 0.0f) ? time : 0.0f; 
        }

        inline void PauseSmooth() noexcept 
        { 
            SmoothPaused = true; 
        }

        inline void ResumeSmooth() noexcept 
        {
            SmoothPaused = false; 
        }

        inline bool IsSmoothPaused() const noexcept 
        { 
            return SmoothPaused; 
        }

        inline bool IsSmoothing() const noexcept
        {
            return !SmoothPaused && SmoothTime > 0.0001f && glm::abs(CurrentProgress - TargetProgress) > 0.001f;
        }

        inline float GetMaxSpeed() const noexcept 
        { 
            return MaxSpeed;
        }

        inline void SetMaxSpeed(float speed) noexcept 
        { 
            MaxSpeed = speed > 0.0f ? speed : 0.0f; 
        }

        inline ProgressDirection GetDirection() const noexcept 
        { 
            return ProgressDirection; 
        }

        inline void SetDirection(ProgressDirection direction) noexcept
        {
            ProgressDirection = direction;
            NeedSyncDirection = true;
        }

        inline bool IsAutoSyncDirection() const noexcept 
        { 
            return AutoSyncDirection; 
        }

        inline void SetAutoSyncDirection(bool enable) noexcept 
        { 
            AutoSyncDirection = enable;
        }

        inline void SetOnComplete(ProgressCompleteCallback callback) noexcept 
        { 
            OnComplete = std::move(callback);
        }

        inline void ClearOnComplete() noexcept 
        { 
            OnComplete = nullptr; 
        }

        inline void SetOnProgressChanged(ProgressChangedCallback callback) noexcept 
        { 
            OnProgressChanged = std::move(callback); 
        }

        inline void ClearOnProgressChanged() noexcept 
        { 
            OnProgressChanged = nullptr; 
        }

        inline void Reset() noexcept 
        { 
            SetProgressImmediate(0.0f);
        }

        inline void Fill() noexcept 
        { 
            SetProgressImmediate(1.0f); 
        }

        inline bool IsComplete() const noexcept 
        { 
            return TargetProgress >= 1.0f && CurrentProgress >= 0.999f; 
        }

        inline bool IsEmpty() const noexcept 
        { 
            return TargetProgress <= 0.0f && CurrentProgress <= 0.001f; 
        }

        inline void SyncRenderOrder(const Sprite* background, Sprite* foreground) noexcept
        {
            Render::RenderQueue bgQueue = background->GetRenderQueue();
            uint8_t bgDepth = background->GetDepth();
            uint8_t bgDrawOrder = background->GetDrawOrder();

            uint8_t fgDrawOrder = bgDrawOrder;
            uint8_t fgDepth = bgDepth;
            Render::RenderQueue fgQueue = bgQueue;

            if (fgDrawOrder < UINT8_MAX)
            {
                fgDrawOrder++;
            }
            else
            {
                fgDrawOrder = 0;
                if (fgDepth < 15)
                {
                    fgDepth++;
                }
                else
                {
                    fgDepth = 0;
                    fgQueue = static_cast<Render::RenderQueue>(
                        static_cast<uint16_t>(fgQueue) + 1
                        );
                }
            }

            foreground->SetRenderQueue(fgQueue);
            foreground->SetDepth(fgDepth);
            foreground->SetDrawOrder(fgDrawOrder);
        }

        inline void SyncDirectionToSprite(Sprite* sprite) noexcept
        {
            if (!sprite) return;
            sprite->SetProgressDirection(ProgressDirection);
        }

        inline void CheckAndTriggerComplete() noexcept
        {
            if (!HasTriggeredComplete && IsComplete())
            {
                HasTriggeredComplete = true;
                if (OnComplete) OnComplete();
            }
        }

        inline void SetIndicatorTransform(Transform* transform) noexcept
        {
            indicatorTransform = transform;
        }

        inline Transform* GetIndicatorTransform() const noexcept
        {
            return indicatorTransform;
        }

        inline bool HasIndicatorAnimation() const noexcept
        {
            return indicatorTransform != nullptr;
        }
    private:
		Sprite* BackgroundSprite = nullptr;
		Sprite* ForegroundSprite = nullptr;
        Transform* indicatorTransform = nullptr;

        float TargetProgress = 0.01f;
        float CurrentProgress = 0.01f;
        float SmoothTime = 0.15f;
        bool SmoothPaused = false;
        float MaxSpeed = 0.0f;

        ProgressDirection ProgressDirection = ProgressDirection::LeftToRight;
        bool AutoSyncDirection = true;
        bool NeedSyncDirection = true;

        ProgressCompleteCallback OnComplete = nullptr;
        ProgressChangedCallback OnProgressChanged = nullptr;
        bool HasTriggeredComplete = false;

        bool ForceForegroundOnTop = false;
        bool NeedSyncRenderOrder = true;
    };
}