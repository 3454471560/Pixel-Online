#pragma once
#include <Config/Common/TileMapID.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <array>

namespace Online::Asset
{
    enum class TextureID : uint8_t
    {
#pragma region default
		Tex_Default = 0,
#pragma endregion

#pragma region HardCode
        Tex_Flag,
        Tex_ProgressBar,
		Tex_ProgressBar_Background,
        Tex_ProgressRun,
        Tex_Loading,
#pragma endregion

#pragma region BackGround
		Tex_BackGround_Near,
		Tex_BackGround_Mid,
		Tex_BackGround_Far,
#pragma endregion

#pragma region Tileset
		Tex_Tileset,
#pragma endregion

#pragma region SilverHat
		Tex_SilverHat_Attack_1,
		Tex_SilverHat_Attack_2,
		Tex_SilverHat_Attack_3,
		Tex_SilverHat_Dash,
		Tex_SilverHat_Dash_Attack,
		Tex_SilverHat_Dash_Attack_Prepare,
		Tex_SilverHat_Death,
		Tex_SilverHat_Fall,
		Tex_SilverHat_Hurt,
		Tex_SilverHat_Idle,
		Tex_SilverHat_Jump,
		Tex_SilverHat_Run,
		Tex_SilverHat_Strong_Attack,
		Tex_SilverHat_Strong_Attack_Prepare,
		Tex_SilverHat_Throw,
#pragma endregion

#pragma region Effect
		Tex_SilverHat_Blade_Effect,
#pragma endregion

#pragma region Off-Screen
		Tex_WindowBuffer,
		Tex_BackBuffer_1,
		Tex_BackBuffer_2,
#pragma endregion

#pragma region TileMap
        Tex_TileMap1,
        Tex_TileMap2,
        Tex_TileMap3,
        Tex_TileMap4,
#pragma endregion

#pragma region Count
		Count
#pragma endregion

    };

    inline TextureID StringToTextureID(const std::string& str)
    {
        static const std::unordered_map<std::string, TextureID> lookup = {
            {"Tex_Default", TextureID::Tex_Default},

            {"Tex_Flag", TextureID::Tex_Flag},
            {"Tex_ProgressBar", TextureID::Tex_ProgressBar},
            {"Tex_ProgressBar_Background", TextureID::Tex_ProgressBar_Background},
            {"Tex_ProgressRun", TextureID::Tex_ProgressRun},
            {"Tex_Loading", TextureID::Tex_Loading},

            {"Tex_BackGround_Near", TextureID::Tex_BackGround_Near},
            {"Tex_BackGround_Mid", TextureID::Tex_BackGround_Mid},
            {"Tex_BackGround_Far", TextureID::Tex_BackGround_Far},

            {"Tex_Tileset", TextureID::Tex_Tileset},

            {"Tex_SilverHat_Attack_1", TextureID::Tex_SilverHat_Attack_1},
            {"Tex_SilverHat_Attack_2", TextureID::Tex_SilverHat_Attack_2},
            {"Tex_SilverHat_Attack_3", TextureID::Tex_SilverHat_Attack_3},
            {"Tex_SilverHat_Dash", TextureID::Tex_SilverHat_Dash},
            {"Tex_SilverHat_Dash_Attack", TextureID::Tex_SilverHat_Dash_Attack},
            {"Tex_SilverHat_Dash_Attack_Prepare", TextureID::Tex_SilverHat_Dash_Attack_Prepare},
            {"Tex_SilverHat_Death", TextureID::Tex_SilverHat_Death},
            {"Tex_SilverHat_Fall", TextureID::Tex_SilverHat_Fall},
            {"Tex_SilverHat_Hurt", TextureID::Tex_SilverHat_Hurt},
            {"Tex_SilverHat_Idle", TextureID::Tex_SilverHat_Idle},
            {"Tex_SilverHat_Jump", TextureID::Tex_SilverHat_Jump},
            {"Tex_SilverHat_Run", TextureID::Tex_SilverHat_Run},
            {"Tex_SilverHat_Strong_Attack", TextureID::Tex_SilverHat_Strong_Attack},
            {"Tex_SilverHat_Strong_Attack_Prepare", TextureID::Tex_SilverHat_Strong_Attack_Prepare},
            {"Tex_SilverHat_Throw", TextureID::Tex_SilverHat_Throw},

            {"Tex_SilverHat_Blade_Effect", TextureID::Tex_SilverHat_Blade_Effect},

            {"Tex_WindowBuffer", TextureID::Tex_WindowBuffer},
            {"Tex_BackBuffer_1", TextureID::Tex_BackBuffer_1},
            {"Tex_BackBuffer_2", TextureID::Tex_BackBuffer_2}
        };

        auto it = lookup.find(str);
        if (it != lookup.end())
            return it->second;

        return TextureID::Tex_Default;
    }
    inline std::string TextureIDToString(TextureID id)
    {
        static constexpr std::array<const char*, static_cast<size_t>(TextureID::Count)> lookup = {
            // default
            "Tex_Default",
            // HardCode
            "Tex_Flag",
            "Tex_ProgressBar",
            "Tex_ProgressBar_Background",
            "Tex_ProgressRun",
            "Tex_Loading",
            // BackGround
            "Tex_BackGround_Near",
            "Tex_BackGround_Mid",
            "Tex_BackGround_Far",
            // Tileset
            "Tex_Tileset",
            // SilverHat
            "Tex_SilverHat_Attack_1",
            "Tex_SilverHat_Attack_2",
            "Tex_SilverHat_Attack_3",
            "Tex_SilverHat_Dash",
            "Tex_SilverHat_Dash_Attack",
            "Tex_SilverHat_Dash_Attack_Prepare",
            "Tex_SilverHat_Death",
            "Tex_SilverHat_Fall",
            "Tex_SilverHat_Hurt",
            "Tex_SilverHat_Idle",
            "Tex_SilverHat_Jump",
            "Tex_SilverHat_Run",
            "Tex_SilverHat_Strong_Attack",
            "Tex_SilverHat_Strong_Attack_Prepare",
            "Tex_SilverHat_Throw",
            // Effect
            "Tex_SilverHat_Blade_Effect",
            // Off-Screen
            "Tex_WindowBuffer",
            "Tex_BackBuffer_1",
            "Tex_BackBuffer_2",
            // TileMap
            "Tex_TileMap1",
            "Tex_TileMap2",
            "Tex_TileMap3",
            "Tex_TileMap4",
        };

        const size_t idx = static_cast<size_t>(id);
        if (idx >= lookup.size())
            return lookup[0]; // Ô½½ç·µ»ØDefault

        std::string name = std::string(lookup[idx]);

        return name;
    }


    inline TextureID TileMapIDToTextureID(Online::Config::TileMapID ID)
    {
        switch (ID)
        {
        case Online::Config::TileMapID::Map_01:
            return TextureID::Tex_TileMap1;
        default:
            return TextureID::Tex_TileMap1;
        }
    }
}