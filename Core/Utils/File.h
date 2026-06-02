#pragma once
#include<windows.h>
#include<stdlib.h>
#include<shlobj.h>
#include<knownfolders.h>

#include<string>
#include<fstream>
#include<filesystem>

namespace Online::Core
{
	inline std::string GetExeDir()
	{
		char path[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, path, MAX_PATH);
		std::string fullPath(path);
		size_t pos = fullPath.find_last_of("\\/");
		return (std::string::npos == pos) ? "" : fullPath.substr(0, pos + 1);
	}

	inline std::string GetPicturesDir()
	{
		PWSTR path = nullptr;
		HRESULT hr = SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &path);

		if (SUCCEEDED(hr))
		{
			int wLen = static_cast<int>(wcslen(path));
			int utf8Size = WideCharToMultiByte(CP_UTF8, 0, path, wLen, nullptr, 0, nullptr, nullptr);
			if (utf8Size <= 0)
			{
				CoTaskMemFree(path);
				return "";
			}
			std::string result(utf8Size, 0);
			WideCharToMultiByte(CP_UTF8, 0, path, wLen, &result[0], utf8Size, nullptr, nullptr);

			CoTaskMemFree(path);
			return result;
		}

		return "";
	}

	inline std::string GetFileNameFromFullPath(const std::string& fullPath)
	{
		if (fullPath.empty())
			return "";

		size_t lastSlashPos = fullPath.find_last_of("/\\");

		if (lastSlashPos == std::string::npos)
			return fullPath;

		return fullPath.substr(lastSlashPos + 1);
	}

	inline bool ReadFileToString(const std::filesystem::path& filepath, std::string& out)
	{
		std::ifstream file(filepath.c_str(), std::ios::binary | std::ios::ate);
		if (!file) { return false; }

		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		out.resize(static_cast<size_t>(size));
		file.read(out.data(), size);

		return file.good();
	}
	inline bool IsSubPath(const std::filesystem::path& path, const std::filesystem::path& base)
	{
		std::filesystem::path absPath = std::filesystem::absolute(path);
		std::filesystem::path absBase = std::filesystem::absolute(base);

		auto parent = absPath.parent_path();
		while (parent != parent.parent_path())
		{
			if (std::filesystem::equivalent(parent, absBase)) { return true; }
			parent = parent.parent_path();
		}
		return false;
	}
}
