#pragma once
#include <Context/Context.h>
#include <Script/Common/ScriptFunctionInfo.h>
#include <stdexcept>

namespace Online::Runtime
{
    template<>
    struct FuncTable<Online::Script::LifeCycleTable>
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
        bool Check() const
        {
            if (!OnRegister) { throw std::runtime_error("FuncTable miss [Script::LifeCycleTable::Register] Function!"); }
            if (!OnGetInfo) { throw std::runtime_error("FuncTable miss [Script::LifeCycleTable::GetInfo] Function!"); }
            return true;
        }

        void UnRegister() noexcept
        {
            OnRegister = nullptr;
            OnGetInfo = nullptr;
        }

    public:
        void InvokeRegister(const Script::ScriptFunctionInfo& info) const noexcept
        {
            OnRegister(info);
        }

        const Script::ScriptFunctionInfo* InvokeGetInfo(Script::ScriptFunctionID id) const noexcept
        {
            return OnGetInfo(id);
        }

    private:
        void(*OnRegister)(const Script::ScriptFunctionInfo&) noexcept = nullptr;
        const Script::ScriptFunctionInfo* (*OnGetInfo)(Script::ScriptFunctionID) noexcept = nullptr;
    };
}

namespace Online::Script
{
    inline void Register(const ScriptFunctionInfo& info) noexcept
    {
        Online::Runtime::Context::Instance().GetFuncTable<LifeCycleTable>().InvokeRegister(info);
    }

    inline const ScriptFunctionInfo* GetInfo(ScriptFunctionID id) noexcept
    {
        return Online::Runtime::Context::Instance().GetFuncTable<LifeCycleTable>().InvokeGetInfo(id);
    }
}