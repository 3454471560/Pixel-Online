#pragma once

#include<cstdint>
#include<string>

namespace Online::Log
{
	enum class LogLevel : uint8_t
	{
		Info,
		Warning,
		Error,
		Debug
	};

	inline std::string_view ToString(LogLevel level) noexcept
	{
		switch (level)
		{
		case Online::Log::LogLevel::Info:		return "Info";
		case Online::Log::LogLevel::Debug:		return "Debug";
		case Online::Log::LogLevel::Warning:	return "Warning";
		case Online::Log::LogLevel::Error:		return "Error";
		default:								return "";
		}
	}
}
