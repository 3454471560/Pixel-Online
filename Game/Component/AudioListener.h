#pragma once
#include <Game/Component/Component.h>

#include <cstdlib> 
#include <glm.hpp>

namespace Online::Game
{
    struct AudioListener : public Component
    {
    public:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("range", range);
            ctx.Write("masterVolume", masterVolume);
            ctx.Write("bgMusicID", static_cast<int>(backgroundMusic));
            ctx.Write("bgMusicVolume", backgroundMusicVolume);
            ctx.Write("bgMusicPaused", backgroundMusicPaused);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            ctx.Read("range", range);
            ctx.Read("masterVolume", masterVolume);
            int mid = 0; ctx.Read("bgMusicID", mid);
            backgroundMusic = static_cast<Asset::MusicID>(mid);
            ctx.Read("bgMusicVolume", backgroundMusicVolume);
            ctx.Read("bgMusicPaused", backgroundMusicPaused);
        }

    public:
        inline float GetRange() noexcept
        {
            return range;
        }
        inline float GetMasterVolume() noexcept
        {
            return masterVolume;
        }
        inline Asset::MusicID GetVackGroundID() noexcept
        {
            return backgroundMusic;
        }
        inline float GetMusicVolum() noexcept
        {
            return backgroundMusicVolume;
        }
        inline float GetPaused() noexcept
        {
            return backgroundMusicPaused;
        }
    public:
        inline void SetRange(float range)
        {
            this->range = range;
        }
        inline void SetMasterVolume(float val)
        {
            masterVolume = val;
        }
        inline void SetMusic(Asset::MusicID ID)
        {
            if (std::rand() % 100 < 5)
            {
                backgroundMusic = Asset::MusicID::Mus_Man;
            }
            else
            {
                backgroundMusic = ID;
            }
        }
        inline void SetMusicVolume(float val)
        {
            backgroundMusicVolume = val;
        }
        inline void SetMusicPause(bool flag)
        {
            backgroundMusicPaused = flag;
        }
    private:
        float range = 100.0f;
        float masterVolume = 1.0f;

        Asset::MusicID backgroundMusic = Asset::MusicID::Invalid;
        float backgroundMusicVolume = 1.0f;
        bool  backgroundMusicPaused = false;
    };
}