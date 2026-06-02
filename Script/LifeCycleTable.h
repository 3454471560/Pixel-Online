#pragma once
#include <Core/Allocate/Allocate.h>
#include <Context/Common/Module.h>
#include <Script/Common/ScriptFunctionInfo.h>

#include <cassert>
#include <array>

namespace Online::Runtime { class Runtime; }

namespace Online::Script
{
    class LifeCycleTable
    {
    public:
        struct Factory
        {
            friend class Online::Runtime::Module<LifeCycleTable>;
        private:
            static LifeCycleTable* Create() { return ONLINE_NEW(LifeCycleTable); }
            static void Destroy(LifeCycleTable* p) { ONLINE_DELETE(p); }
        };

        struct Lifecycle
        {
            friend class Online::Runtime::Module<LifeCycleTable>;
        private:
            static bool Initialize(LifeCycleTable* p) { return p->Initialize(); }
            static void Release(LifeCycleTable* p) { p->Release(); }
        };

    private:
        LifeCycleTable() = default;
        ~LifeCycleTable() = default;

    public:
        LifeCycleTable(const LifeCycleTable&) = delete;
        LifeCycleTable& operator=(const LifeCycleTable&) = delete;
        LifeCycleTable(LifeCycleTable&&) = delete;
        LifeCycleTable& operator=(LifeCycleTable&&) = delete;
    public:
        bool Initialize();
        void Release();

        void Register(const ScriptFunctionInfo& info);

        const ScriptFunctionInfo* GetInfo(ScriptFunctionID id) const;

    private:
        std::array<ScriptFunctionInfo, static_cast<size_t>(ScriptFunctionID::Count)> registry;
    };
}