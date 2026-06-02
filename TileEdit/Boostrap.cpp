#include <TileEdit/Boostrap.h>
#include <TileEdit/TileEdit.h>

#include <windows.h>

#include <exception>
#include <stdexcept>
#include <filesystem>

LONG WINAPI CrashFilter(EXCEPTION_POINTERS*)
{
	Online::TileEdit::TileEdit::Instance().Terminate();
	return EXCEPTION_EXECUTE_HANDLER;
}

int Online::TileEdit::Execute(int argc, char** argv)
{
	try
	{
		SetUnhandledExceptionFilter(CrashFilter);

		int32_t exitcode = Online::TileEdit::TileEdit::Instance().Execute();
		return exitcode;
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_OK);
		Online::TileEdit::TileEdit::Instance().Terminate();
		return -1;
	}
	catch (...)
	{
		MessageBoxA(NULL, "Unknown error!", "Error", MB_OK);
		Online::TileEdit::TileEdit::Instance().Terminate();
		return -1;
	}
}

void Online::TileEdit::Terminate()
{
	Online::TileEdit::TileEdit::Instance().Terminate();
}
