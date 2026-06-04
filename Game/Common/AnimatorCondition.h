#pragma once
#include <Game/Common/AnimatorConditionMode.h>
#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>

#include <cstdint>

namespace Online::Game
{
    struct AnimatorCondition
    {
        std::string ParameterName;
        AnimatorConditionMode Mode = AnimatorConditionMode::If;
        float Threshold = 0.0f;

        void Serialize(Online::Serialize::SerializeContext& ctx) const
        {
            ctx.Write("param", ParameterName);
            ctx.Write("mode", static_cast<uint8_t>(Mode));
            ctx.Write("threshold", Threshold);
        }
        void Deserialize(const Online::Serialize::DeserializeContext& ctx)
        {
            ctx.Read("param", ParameterName);
            uint8_t m; ctx.Read("mode", m); Mode = static_cast<AnimatorConditionMode>(m);
            ctx.Read("threshold", Threshold);
        }
    };
}