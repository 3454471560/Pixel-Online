#pragma once

#include<Context/Context.h>
#include<Client/Context/ClientContext.h>
#include<Log/Common/LogLevel.h>
#include<stdexcept>

namespace Online::Runtime
{
	template<>
	struct FuncTable<Online::Log::Logger>
	{
		friend class Online::Runtime::Client;
		friend class Online::Runtime::Server;
	private:
		FuncTable() = default;
		~FuncTable() = default;

	public:
		FuncTable(const FuncTable&) = delete;
		FuncTable& operator=(const FuncTable&) = delete;
		FuncTable(FuncTable&&) = delete;
		FuncTable& operator=(FuncTable&&) = delete;

	public:
		inline bool Check() const
		{
			if (!OnLog) { throw std::runtime_error("Delegates miss [Log::Log] Function!"); }
			return true;
		}
		inline void UnRegister() noexcept
		{
			OnLog = nullptr;
		}

	public:
		inline void InvokeOnLog(Online::Log::LogLevel level, std::string_view info) noexcept
		{
			OnLog(level, info);
		}

	private:
		void (*OnLog)(Online::Log::LogLevel, std::string_view) noexcept = nullptr;
	};
}

namespace Online::Log
{
	inline void Write(Online::Log::LogLevel level, std::string_view info) noexcept
	{
		Online::Runtime::Context::Instance().GetFuncTable<Online::Log::Logger>().InvokeOnLog(level, info);
	}
	inline void Info(std::string_view info) noexcept
	{
		Online::Runtime::Context::Instance().GetFuncTable<Online::Log::Logger>().InvokeOnLog(Online::Log::LogLevel::Info, info);
	}
	inline void Debug(std::string_view info) noexcept
	{
		Online::Runtime::Context::Instance().GetFuncTable<Online::Log::Logger>().InvokeOnLog(Online::Log::LogLevel::Debug, info);
	}
	inline void Warning(std::string_view info) noexcept
	{
		Online::Runtime::Context::Instance().GetFuncTable<Online::Log::Logger>().InvokeOnLog(Online::Log::LogLevel::Warning, info);
	}
	inline void Error(std::string_view info) noexcept
	{
		Online::Runtime::Context::Instance().GetFuncTable<Online::Log::Logger>().InvokeOnLog(Online::Log::LogLevel::Error, info);
	}
}
