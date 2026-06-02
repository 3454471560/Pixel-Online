#pragma once

#include<string>
#include<chrono>

namespace Online::Core
{
	inline std::string Date();
	inline std::string Data(std::chrono::system_clock::time_point);
	inline std::string Data(time_t);
	inline std::uint32_t CurrentMs();
	std::string Online::Core::Date()
	{
		auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		struct tm buf;
		localtime_s(&buf, &t);
		char temp[32] = {};
		std::strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S", &buf);
		return temp;
	}
	std::string Online::Core::Data(std::chrono::system_clock::time_point tp)
	{
		auto t = std::chrono::system_clock::to_time_t(tp);
		struct tm buf;
		localtime_s(&buf, &t);
		char temp[32] = {};
		std::strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S", &buf);
		return temp;
	}
	std::string Online::Core::Data(time_t tp)
	{
		struct tm buf;
		localtime_s(&buf, &tp);
		char temp[32] = {};
		std::strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S", &buf);
		return temp;
	}
	uint32_t CurrentMs()
	{
		return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count());
	}

}
