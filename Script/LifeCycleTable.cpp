#include <Script/LifeCycleTable.h>
#include <Script/LifeCycleFunc/MoveLeftRight.h> 
#include <Script/LifeCycleFunc/Rotate.h> 

namespace Online::Script
{
    bool LifeCycleTable::Initialize()
    {
        registry.fill(ScriptFunctionInfo{});
        Script::Register(Move::Information());
        Script::Register(Rotate::Information());
        return true;
    }

    void LifeCycleTable::Release()
    {
	}
    void LifeCycleTable::Register(const ScriptFunctionInfo& info)
    {
        assert(info.id < ScriptFunctionID::Count);
		registry[static_cast<size_t>(info.id)] = info;
	}
    const ScriptFunctionInfo* LifeCycleTable::GetInfo(ScriptFunctionID id) const
    {
        const auto idx = static_cast<size_t>(id);
        return (idx < registry.size()) ? &registry[idx] : nullptr;
    }
}