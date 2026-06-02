#pragma once
#include <Windows.h>
#include <Commdlg.h>

#include <string>
namespace Online::TileEdit::Dialog
{
	std::string OpenFileDialog()
	{
		char filePath[MAX_PATH] = { 0 };

		OPENFILENAMEA ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "地图文件 (*.csv)\0*.csv\0所有文件 (*.*)\0*.*\0";
		ofn.lpstrFile = filePath;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.lpstrTitle = "打开地图文件";

		if (GetOpenFileNameA(&ofn))
		{
			return std::string(filePath);
		}

		return "";
	}

	std::string SaveFileCsvDialog()
	{
		char filePath[MAX_PATH] = { 0 };

		OPENFILENAMEA ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "地图文件 (*.csv)\0*.csv\0所有文件 (*.*)\0*.*\0";
		ofn.lpstrFile = filePath;
		ofn.nMaxFile = MAX_PATH;

		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT;
		ofn.lpstrTitle = "保存地图文件";

		if (GetSaveFileNameA(&ofn))
		{
			return std::string(filePath);
		}

		return "";
	}

	std::string SaveFilePngDialog()
	{
		char filePath[MAX_PATH] = { 0 };

		OPENFILENAMEA ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "地图文件 (*.png)\0*.png\0所有文件 (*.*)\0*.*\0";
		ofn.lpstrFile = filePath;
		ofn.nMaxFile = MAX_PATH;

		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXTENSIONDIFFERENT;
		ofn.lpstrTitle = "保存PNG文件";

		if (GetSaveFileNameA(&ofn))
		{
			return std::string(filePath);
		}

		return "";
	}
}
