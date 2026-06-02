#pragma once
#include <Audio/Common/AudioQueue.h>

namespace Online::Audio
{
	struct ChannelInfo
	{
		uint32_t id;
	    AudioQueue priority = AudioQueue(0);
	};
}