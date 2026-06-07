#pragma once
#include <cstdint>

namespace Online::Script
{
	enum class ButtonState : uint8_t
	{ 
		Normal, 
		Hovered, 
		Pressed 
	};
}
