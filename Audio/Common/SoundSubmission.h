#pragma once
#include <Asset/Common/ID/SoundID.h>
#include <Audio/Common/AudioQueue.h>

#include <glm.hpp>

namespace Online::Audio
{
	struct SoundSubmission
	{
        uint32_t id;
        glm::vec2 worldPos;
        Asset::SoundID soundId;
        float volume;
        bool loop;
        bool isPlaying;
        float spatialBlend;
        AudioQueue priority;
	};
}