#pragma once

#include<string>  
#include<algorithm>
#include<cctype>
#include<cstring>

namespace Online::Core
{
	inline constexpr char32_t REPLACEMENT_CHAR = 0xFFFDU;
	inline constexpr char32_t MAX_UNICODE_CP = 0x10FFFFU;
	inline constexpr char32_t SURROGATE_BEGIN = 0xD800U;
	inline constexpr char32_t SURROGATE_END = 0xDFFFU;

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

	inline std::u32string Utf8ToUtf32(const std::string & utf8)
	{
		std::u32string result;
		const uint8_t* src = reinterpret_cast<const uint8_t*>(utf8.data());
		const size_t len = utf8.size();
		size_t idx = 0;

		while (idx < len)
		{
			const uint8_t b0 = src[idx];
			size_t byteCount = 0;
			char32_t cp = 0;

			if ((b0 & 0x80U) == 0x00U)
			{
				byteCount = 1;
				cp = b0;
			}
			else if ((b0 & 0xE0U) == 0xC0U)
			{
				byteCount = 2;
				cp = b0 & 0x1FU;
			}
			else if ((b0 & 0xF0U) == 0xE0U)
			{
				byteCount = 3;
				cp = b0 & 0x0FU;
			}
			else if ((b0 & 0xF8U) == 0xF0U)
			{
				byteCount = 4;
				cp = b0 & 0x07U;
			}
			else
			{
				result.push_back(REPLACEMENT_CHAR);
				idx += 1;
				continue;
			}

			if (idx + byteCount > len)
			{
				result.push_back(REPLACEMENT_CHAR);
				idx += 1;
				continue;
			}

			bool invalidSeq = false;
			for (size_t i = 1; i < byteCount; ++i)
			{
				const uint8_t subByte = src[idx + i];
				if ((subByte & 0xC0U) != 0x80U)
				{
					invalidSeq = true;
					break;
				}
				cp = (cp << 6) | (subByte & 0x3FU);
			}

			if (invalidSeq)
			{
				result.push_back(REPLACEMENT_CHAR);
				idx += 1;
				continue;
			}

			bool overLong = false;
			switch (byteCount)
			{
			case 2: overLong = (cp < 0x80U); break;
			case 3: overLong = (cp < 0x800U); break;
			case 4: overLong = (cp < 0x10000U); break;
			default: break;
			}

			bool cpIllegal = (cp > MAX_UNICODE_CP) || (cp >= SURROGATE_BEGIN && cp <= SURROGATE_END);

			if (overLong || cpIllegal)
			{
				result.push_back(REPLACEMENT_CHAR);
				idx += 1;
				continue;
			}

			result.push_back(cp);
			idx += byteCount;
		}

		return result;
	}

	inline size_t CodepointToUtf8(char32_t cp, char out[4]) noexcept
	{
		if (cp > MAX_UNICODE_CP || (cp >= SURROGATE_BEGIN && cp <= SURROGATE_END))
		{
			cp = REPLACEMENT_CHAR;
		}

		uint8_t* dst = reinterpret_cast<uint8_t*>(out);
		if (cp <= 0x7FU)
		{
			dst[0] = static_cast<uint8_t>(cp);
			return 1;
		}
		else if (cp <= 0x7FFU)
		{
			dst[0] = static_cast<uint8_t>(0xC0U | ((cp >> 6) & 0x1FU));
			dst[1] = static_cast<uint8_t>(0x80U | (cp & 0x3FU));
			return 2;
		}
		else if (cp <= 0xFFFFU)
		{
			dst[0] = static_cast<uint8_t>(0xE0U | ((cp >> 12) & 0x0FU));
			dst[1] = static_cast<uint8_t>(0x80U | ((cp >> 6) & 0x3FU));
			dst[2] = static_cast<uint8_t>(0x80U | (cp & 0x3FU));
			return 3;
		}
		else
		{
			dst[0] = static_cast<uint8_t>(0xF0U | ((cp >> 18) & 0x07U));
			dst[1] = static_cast<uint8_t>(0x80U | ((cp >> 12) & 0x3FU));
			dst[2] = static_cast<uint8_t>(0x80U | ((cp >> 6) & 0x3FU));
			dst[3] = static_cast<uint8_t>(0x80U | (cp & 0x3FU));
			return 4;
		}
	}

	inline std::string Utf32ToUtf8(std::u32string_view u32Str)
	{
		std::string dst;
		dst.reserve(u32Str.size() * 4U);

		char temp[4]{};
		for (char32_t cp : u32Str)
		{
			size_t writeLen = CodepointToUtf8(cp, temp);
			dst.append(temp, writeLen);
		}
		return dst;
	}

	inline std::string Utf32ToUtf8(const std::u32string& u32Str)
	{
		return Utf32ToUtf8(std::u32string_view(u32Str));
	}

}
