#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

namespace Online::Asset
{
    enum class AnimationClipID : uint8_t
    {
#pragma region HardCode
        Anim_Flag,
        Anim_ProgressRun,
        Anim_Loading,
#pragma endregion

#pragma region SilverHat
		Anim_SilverHat_Attack_1,
		Anim_SilverHat_Attack_2,
		Anim_SilverHat_Attack_3,
		Anim_SilverHat_Dash,
		Anim_SilverHat_Dash_Attack,
		Anim_SilverHat_Dash_Attack_Prepare,
		Anim_SilverHat_Death,
		Anim_SilverHat_Fall,
		Anim_SilverHat_Hurt,
		Anim_SilverHat_Idle,
		Anim_SilverHat_Jump,
		Anim_SilverHat_Run,
		Anim_SilverHat_Strong_Attack,
		Anim_SilverHat_Strong_Attack_Prepare,
		Anim_SilverHat_Throw,
#pragma endregion

        Count
    };

    inline AnimationClipID StringToAnimationClipID(const std::string& str)
    {
        static const std::unordered_map<std::string, AnimationClipID> lookup = {
            {"Anim_Flag",AnimationClipID::Anim_Flag},
            {"Anim_ProgressRun",AnimationClipID::Anim_ProgressRun},
            {"Anim_SilverHat_Attack_1", AnimationClipID::Anim_SilverHat_Attack_1},
            {"Anim_SilverHat_Attack_2", AnimationClipID::Anim_SilverHat_Attack_2},
            {"Anim_SilverHat_Attack_3", AnimationClipID::Anim_SilverHat_Attack_3},
            {"Anim_SilverHat_Dash", AnimationClipID::Anim_SilverHat_Dash},
            {"Anim_SilverHat_Dash_Attack", AnimationClipID::Anim_SilverHat_Dash_Attack},
            {"Anim_SilverHat_Dash_Attack_Prepare", AnimationClipID::Anim_SilverHat_Dash_Attack_Prepare},
            {"Anim_SilverHat_Death", AnimationClipID::Anim_SilverHat_Death},
            {"Anim_SilverHat_Fall", AnimationClipID::Anim_SilverHat_Fall},
            {"Anim_SilverHat_Hurt", AnimationClipID::Anim_SilverHat_Hurt},
            {"Anim_SilverHat_Idle", AnimationClipID::Anim_SilverHat_Idle},
            {"Anim_SilverHat_Jump", AnimationClipID::Anim_SilverHat_Jump},
            {"Anim_SilverHat_Run", AnimationClipID::Anim_SilverHat_Run},
            {"Anim_SilverHat_Strong_Attack", AnimationClipID::Anim_SilverHat_Strong_Attack},
            {"Anim_SilverHat_Strong_Attack_Prepare", AnimationClipID::Anim_SilverHat_Strong_Attack_Prepare},
            {"Anim_SilverHat_Throw", AnimationClipID::Anim_SilverHat_Throw}
        };

        auto it = lookup.find(str);
        if (it != lookup.end())
            return it->second;

        return AnimationClipID::Count;
    }

    inline AnimationClipID StringToAnimationClipID(const char* str)
    {
        return StringToAnimationClipID(std::string(str ? str : ""));
    }
}
