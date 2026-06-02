#include <Client/Boostrap.h>
#include <Client/Client.h>

#include <windows.h>

#include <exception>
#include <stdexcept>
#include <filesystem>

LONG WINAPI CrashFilter(EXCEPTION_POINTERS*)
{
	Online::Client::Terminate();
	return EXCEPTION_EXECUTE_HANDLER;
}

int Online::Client::Execute(int argc, char** argv)
{
	try
	{
		SetUnhandledExceptionFilter(CrashFilter);

		int32_t exitcode = Online::Runtime::Client::Instance().Execute();
		return exitcode;
	}
	catch (const std::exception& e)
	{
		MessageBoxA(NULL, e.what(), "Error", MB_OK);
		Online::Runtime::Client::Instance().Terminate();
		return -1;
	}
	catch (...)
	{
		MessageBoxA(NULL, "Unknown error!", "Error", MB_OK);
		Online::Runtime::Client::Instance().Terminate();
		return -1;
	}
}

void Online::Client::Terminate()
{
	Online::Runtime::Client::Instance().Terminate();
}
