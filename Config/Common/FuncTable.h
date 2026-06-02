#pragma once

#include<Context/Context.h>
#include<Client/Context/ClientContext.h>
#include<Render/Common/API.h>
#include<TileEdit/Common/TileMap.h>
#include<Config/Common/TileMapID.h>

#include<stdexcept>

namespace Online::Runtime
{
	template<>
	struct FuncTable<Online::Config::Configurator>
	{
		friend class Online::Runtime::Client;
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
			if (!OnGetRenderAPI) { throw std::runtime_error("FuncTable miss [Config::GetRenderAPI] Function!"); }
			if (!OnGetEnableVSync) { throw std::runtime_error("FuncTable miss [Config::GetEnableVSync] Function!"); }
			if (!GetTileMap) { throw std::runtime_error("FuncTable miss [Config::GetTileMap] Function!"); }
			return true;
		}
		inline void UnRegister() noexcept
		{
			OnGetRenderAPI = nullptr;
			OnGetMaxFixupdataExecuteTimes = nullptr;
			OnGetEnableVSync = nullptr;
			GetTileMap = nullptr;
		}

	public:
		inline Online::Render::API InvokeOnGetRenderAPI() noexcept
		{
			return OnGetRenderAPI();
		}
		inline size_t InvokeOnGetMaxFixupdataExecuteTimes() noexcept
		{
			return OnGetMaxFixupdataExecuteTimes();
		}
		inline bool InvokeOnGetEnableVSync() const noexcept
		{
			return OnGetEnableVSync();
		}
		inline const Online::TileEdit::TileMap& InvokeGetTileMap(Online::Config::TileMapID ID) const noexcept
		{
			return GetTileMap(ID);
		}

	private:
		Online::Render::API(*OnGetRenderAPI)() noexcept = nullptr;
		size_t(*OnGetMaxFixupdataExecuteTimes)() noexcept = nullptr;
		bool(*OnGetEnableVSync)() noexcept = nullptr;
		const Online::TileEdit::TileMap& (*GetTileMap)(Online::Config::TileMapID) noexcept = nullptr;
	};
}

namespace Online::Config
{
	inline Online::Render::API GetRenderAPI() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Config::Configurator>().InvokeOnGetRenderAPI();
	}
	inline size_t GetMaxFixupdataExecuteTimes() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Config::Configurator>().InvokeOnGetMaxFixupdataExecuteTimes();
	}
	inline bool GetEnableVSync() noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Config::Configurator>().InvokeOnGetEnableVSync();
	}
	inline const Online::TileEdit::TileMap& GetTileMap(Online::Config::TileMapID ID) noexcept
	{
		return Online::Runtime::ClientContext::Instance().GetClientFuncTable<Online::Config::Configurator>().InvokeGetTileMap(ID);
	}
}
