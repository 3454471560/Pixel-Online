#pragma once
#include <Asset/Common/ID/AnimationClipID.h>
#include <Asset/Common/Data/AnimationClip.h>
#include <Asset/Common/FuncTable.h>
#include <Game/Common/KeyframeEvent.h>
#include <Game/Component/Sprite.h>
#include <Game/Component/Component.h>
#include <Log/Common/FuncTable.h>

#include <functional>

namespace Online::Game
{
	struct Animator : public Component
    {
        static constexpr int MAX_KEYFRAME_EVENT = 8;

        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("clipID", static_cast<int>(CurrentClipID));
            ctx.Write("currentTime", currentTime);
            ctx.Write("playing", Playing);
            ctx.Write("paused", Paused);
            ctx.Write("needApplySettings", NeedApplySettings);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            int cid = 0; ctx.Read("clipID", cid);
            CurrentClipID = static_cast<Asset::AnimationClipID>(cid);
            ctx.Read("currentTime", currentTime);
            ctx.Read("playing", Playing);
            ctx.Read("paused", Paused);
            ctx.Read("needApplySettings", NeedApplySettings);
            CurrentFrameIndex = 0xFF;
        }
    public:
        inline void SetSprite(Sprite* sprite)
        {
            if (sprite->gameObject != this->gameObject)
            {
                Log::Error("Sprite Component Have To with Animator Component In same GameObject");
                return;
            }
            this->sprite = sprite;
        }

        inline Sprite* GetSprite()
        {
            return sprite;
        }

        inline Sprite* GetSprite() const
        {
            return sprite;
        }

        inline bool IsPlaying() const noexcept
        {
            return Playing;
        }

        inline void SetPlaying(bool flag) noexcept
        {
            Playing = flag;
        }

        inline bool IsPaused() const noexcept
        {
            return Paused;
        }

        inline bool GetNeedApplySettings() const noexcept
        {
            return NeedApplySettings;
        }

        inline void SetNeedApplySettings(bool flag)
        {
            NeedApplySettings = flag;
        }

        inline float CurrentTime()
        {
            return this->currentTime;
        }

        inline void SetCurrentTime(float time)
        {
            currentTime = time;
        }

        inline void AddCurrentTime(float time)
        {
            currentTime += time;
        }

        inline void SubCurrentTime(float time)
        {
            currentTime -= time;
        }

        inline float GetCurrentTime() const noexcept
        {
            return currentTime;
        }

        inline uint8_t GetCurrentFrameIndex() const noexcept
        {
            return CurrentFrameIndex;
        }

        inline void SetCurrentFrameIndex(uint8_t idx) noexcept
        {
            CurrentFrameIndex = idx;
        }

        inline Asset::AnimationClipID GetCurrentClipID() const noexcept
        {
            return CurrentClipID;
        }

        inline const Asset::AnimationClip* GetCurrentClip() const noexcept
        {
            return Online::Asset::GetAnim(CurrentClipID);
        }

        inline void Play(Asset::AnimationClipID clipID, bool restart = true) noexcept
        {
            if (!restart && CurrentClipID == clipID)
                return;

            CurrentClipID = clipID;
            currentTime = 0.0f;
            CurrentFrameIndex = 0xFF;
            NeedApplySettings = true;

            Playing = true;
            Paused = false;
        }

        inline void Pause() noexcept
        {
            Paused = true;
        }

        inline void Resume() noexcept
        {
            Paused = false;
        }

        inline void Stop() noexcept
        {
            Playing = false;
            Paused = false;
            currentTime = 0.0f;
            CurrentFrameIndex = 0xFF;
        }

        inline void ApplyClipSettings(const Asset::AnimationClip* clip, Sprite* sprite) noexcept
        {
            if (!clip || !sprite) return;

            sprite->SetTexture(clip->GetTextureID());
            sprite->SetGrid(clip->GetGridCols(), clip->GetGridRows());
        }

        inline void SetFlipX(bool flipX) noexcept
        {
            if (!sprite) return;
            sprite->SetFlipX(flipX);
		}
        inline void SetFlipY(bool flipY) noexcept
        {
            if (!sprite) return;
            sprite->SetFlipY(flipY);
		}

        inline void AddKeyframe(uint8_t frameIndex, std::function<void()> Event)
        {
            for (int i = 0; i < MAX_KEYFRAME_EVENT; ++i)
            {
                if (keyframeEvents[i].frameIdx == 0xFF)
                {
                    keyframeEvents[i].frameIdx = frameIndex;
                    keyframeEvents[i].Event = std::move(Event);
                    return;
                }
            }
            Log::Warning("Animator keyframe event array full, max count: 8");
        }

        inline void RemoveKeyframe(uint8_t frameIndex)
        {
            for (int i = 0; i < MAX_KEYFRAME_EVENT; ++i)
            {
                if (keyframeEvents[i].frameIdx == frameIndex)
                {
                    keyframeEvents[i].Clear();
                    break;
                }
            }
        }

        inline void ClearAllKeyframeEvents()
        {
            for (int i = 0; i < MAX_KEYFRAME_EVENT; ++i)
            {
                keyframeEvents[i].Clear();
            }
        }

        void ExecuteKeyframeEvents(uint8_t curFrame)
        {
            for (int i = 0; i < MAX_KEYFRAME_EVENT; ++i)
            {
                auto& evt = keyframeEvents[i];
                if (evt.frameIdx == curFrame && evt.Event)
                {
                    evt.Event();
                }
            }
        }

    private:
        Asset::AnimationClipID CurrentClipID = static_cast<Asset::AnimationClipID>(0);
        Game::Sprite* sprite = nullptr;
        float currentTime = 0.0f;
        uint8_t CurrentFrameIndex = 0xFF;
        bool Playing = false;
        bool Paused = false;
        bool NeedApplySettings = true;

        KeyframeEvent keyframeEvents[MAX_KEYFRAME_EVENT];
    };
}