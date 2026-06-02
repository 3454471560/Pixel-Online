#pragma once
#include <Asset/Common/ID/SoundID.h>
#include <Audio/Common/AudioQueue.h>

#include <glm.hpp>

#include <cstdint>

namespace Online::Audio
{
    struct ActiveSound
    {
        uint32_t id;
        Asset::SoundID soundId = Asset::SoundID::Invalid;
        float baseVolume = 1.0f;
        float spatialBlend = 0.0f;
        bool loop = false;
        AudioQueue priority = AudioQueue::WorldSFX;
        glm::vec2 worldPos = { 0.0f, 0.0f };
        int channel = -1;
        bool visitedThisFrame = false;
    };

}