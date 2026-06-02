#pragma once

#include <string>
#include <vector>

namespace Online::Config
{
	struct AnimationInfo
	{
		std::string name;
		int grid[2] = { 0, 0 };
		int frameRate = 0;
		bool looping = false;
		std::vector<int> frames;
	};
}