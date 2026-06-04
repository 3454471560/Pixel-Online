#pragma once
#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>

namespace Online::Game
{
    struct AnimatorParameter
    {
        std::string Name;
        AnimatorParameterType Type = AnimatorParameterType::Float;
        float FloatValue = 0.0f;

        void Serialize(Online::Serialize::SerializeContext& ctx) const
        {
            ctx.Write("name", Name);
            ctx.Write("type", static_cast<uint8_t>(Type));
            ctx.Write("value", FloatValue);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx)
        {
            ctx.Read("name", Name);
            uint8_t t; ctx.Read("type", t); Type = static_cast<AnimatorParameterType>(t);
            ctx.Read("value", FloatValue);
        }
    };
}