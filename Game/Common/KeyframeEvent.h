#pragma once
#include <cstdint>
#include <functional>

struct KeyframeEvent
{
	uint8_t frameIdx;
	std::function<void()> Event;

    void Clear()
    {
        frameIdx = 0xFF;
        Event = nullptr;
    }
};