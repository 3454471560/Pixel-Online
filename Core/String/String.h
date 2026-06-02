#pragma once

#include<string>  
#include<algorithm>
#include<cctype>
#include<cstring>

namespace Online::Core
{
	inline std::string ToLower(std::string_view strview) noexcept
	{
		std::string str = std::string(strview);
		std::transform(str.begin(), str.end(), str.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return str;
	}
	inline std::string ToUpper(std::string_view strview) noexcept
	{
		std::string str = std::string(strview);
		std::transform(str.begin(), str.end(), str.begin(),
			[](unsigned char c) { return std::toupper(c); });
		return str;
	}
	inline void ToLower(std::string& str) noexcept
	{
		std::transform(str.begin(), str.end(), str.begin(),
			[](unsigned char c) { return std::tolower(c); });
	}
	inline void ToUpper(std::string& str) noexcept
	{
		std::transform(str.begin(), str.end(), str.begin(),
			[](unsigned char c) { return std::toupper(c); });
	}

	template <size_t N>
	inline void ToUpper(char(&arr)[N]) noexcept
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (arr[i] == '\0') { break; }
			arr[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(arr[i])));
		}
	}
	template <size_t N>
	inline void ToLower(char(&arr)[N]) noexcept
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (arr[i] == '\0') { break; }
			arr[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(arr[i])));
		}
	}

	inline void CopyStringToBuffer(std::string_view sv, char* buffer, size_t size)
	{
		if (buffer == nullptr || size == 0) { return; }

		const size_t copyLength = (std::min<size_t>)(sv.size(), size - 1);
		std::memcpy(buffer, sv.data(), copyLength);
		buffer[copyLength] = '\0';
	}

	template <size_t N>
	inline void CopyStringToCharArray(std::string_view sv, char(&arr)[N])
	{
		size_t copyLength = (std::min<size_t>)(sv.size(), N - 1);
		std::memcpy(arr, sv.data(), copyLength);
		arr[copyLength] = '\0';
	}

	inline std::string Prepend(const std::string& prefix, const std::string& str)
	{
		return prefix + str;
	}

	inline std::string Prepend(const char* prefix, const std::string& str)
	{
		return std::string(prefix) + str;
	}

	inline std::string Prepend(const std::string& prefix, const char* str)
	{
		return prefix + std::string(str);
	}

	inline void PrependInPlace(const std::string& prefix, std::string& str)
	{
		str.insert(0, prefix);
	}

	inline void PrependInPlace(const char* prefix, std::string& str)
	{
		str.insert(0, prefix);
	}

	inline void BuildCharArrFormString(const std::string& str, char* buffer, size_t bufferSize)
	{
		if (!buffer || bufferSize == 0)
			return;

		strncpy_s(buffer, bufferSize, str.c_str(), _TRUNCATE);
	}

	inline void BuildCharArrFormString(const char* str, char* buffer, size_t bufferSize)
	{
		if (!buffer || bufferSize == 0)
			return;
		strncpy_s(buffer, bufferSize, str ? str : "", _TRUNCATE);
	}
}
