#pragma once
#include <Core/StateFlags/StateFlags.h>
#include <Audio/Common/AudioQueue.h>
#include <Asset/Common/ID/SoundID.h>
#include <Asset/Common/ID/MusicID.h>
#include <Game/Component/Component.h>
#include <variant>
#include <glm.hpp>

namespace Online::Game
{
    struct AudioSource : public Component
    {
    public:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("soundID", static_cast<int>(SoundID));
            ctx.Write("volume", Volume);
            ctx.Write("loop", Loop);
            ctx.Write("playing", IsPlaying);
            ctx.Write("spatialBlend", SpatialBlend);
            ctx.Write("priority", static_cast<uint8_t>(Priority));
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            int sid = 0; ctx.Read("soundID", sid); SoundID = static_cast<Asset::SoundID>(sid);
            ctx.Read("volume", Volume);
            ctx.Read("loop", Loop);
            ctx.Read("playing", IsPlaying);
            ctx.Read("spatialBlend", SpatialBlend);
            uint8_t prio = 0; ctx.Read("priority", prio);
            Priority = static_cast<Audio::AudioQueue>(prio);
        }

    public:
        inline Asset::SoundID GetSoundID() const noexcept 
        { 
            return SoundID; 
        }
        inline void SetSoundID(Asset::SoundID id) noexcept 
        { 
            SoundID = id;
        }

        inline float GetVolume() const noexcept 
        { 
            return Volume;
        }
        inline void SetVolume(float vol) noexcept 
        { 
            Volume = std::clamp(vol, 0.0f, 1.0f); 
        }

        inline bool IsLoop() const noexcept 
        { 
            return Loop; 
        }
        inline void SetLoop(bool loop) noexcept 
        { 
            Loop = loop; 
        }

        inline bool GetPlaying() const noexcept 
        { 
            return IsPlaying; 
        }
        inline void SetPlaying(bool playing) noexcept 
        { 
            this->IsPlaying = playing; 
        }

        inline float GetSpatialBlend() const noexcept 
        { 
            return SpatialBlend; 
        }
        inline void SetSpatialBlend(float blend) noexcept 
        { 
            SpatialBlend = std::clamp(blend, 0.0f, 1.0f); 
        }

        inline Audio::AudioQueue GetPriority() const noexcept 
        { 
            return Priority; 
        }
        inline void SetPriority(Audio::AudioQueue priority) noexcept 
        { 
            Priority = priority; 
        }

    private:
        Asset::SoundID SoundID = Asset::SoundID::Invalid;
        float Volume = 1.0f;
        bool  Loop = false;
        bool  IsPlaying = false;
        float SpatialBlend = 1.0f;          // 0=2D, 1=3D
        Audio::AudioQueue Priority = Audio::AudioQueue::WorldSFX;
    };
}