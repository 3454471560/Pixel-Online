#pragma once

#include<cstdint>

namespace Online::Serialize
{
	enum class API : uint8_t
	{
		MsgPack = 0,
		Json = 1,

		Unknown
	};
}
