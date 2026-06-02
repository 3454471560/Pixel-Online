#pragma once

#include<cstdint>
#include<string>

namespace Online::Render
{
	enum class RenderQueue : uint16_t
	{
		Background = 1000,
		World = 2000,
		Effects = 3000,
		UI = 4000,
		Overlay = 5000
	};

    inline std::string ToString(RenderQueue renderQueue) noexcept
    {
        switch (renderQueue)
        {
        case RenderQueue::Background: return "Background";
        case RenderQueue::World:      return "World";
        case RenderQueue::Effects:    return "Effects";
        case RenderQueue::UI:         return "UI";
        case RenderQueue::Overlay:    return "Overlay";
        default:                      return "Unknown";
        }
    }
}
