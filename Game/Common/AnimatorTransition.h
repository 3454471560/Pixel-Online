#pragma once
#include <Game/Common/AnimatorCondition.h>
#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>

namespace Online::Game
{

    struct AnimatorTransition
    {
        std::string SourceState;
        std::string DestState;
        float Duration = 0.25f;
        std::vector<AnimatorCondition> Conditions;

        void Serialize(Online::Serialize::SerializeContext& ctx) const
        {
            ctx.Write("src", SourceState);
            ctx.Write("dst", DestState);
            ctx.Write("duration", Duration);
            ctx.BeginArray("conditions");
            for (const auto& c : Conditions)
            {
                Serialize::SerializeContext& cc = ctx.WriteArrayObjectBegin();
                c.Serialize(cc);
            }
            ctx.EndArray();
        }
        void Deserialize(const Online::Serialize::DeserializeContext& ctx)
        {
            ctx.Read("src", SourceState);
            ctx.Read("dst", DestState);
            ctx.Read("duration", Duration);
            const auto& arr = ctx.GetSubContext("conditions");
            size_t cnt = 0;
            if (arr.GetArraySize("", cnt))
            {
                Conditions.resize(cnt);
                for (size_t i = 0; i < cnt; ++i)
                    Conditions[i].Deserialize(arr.GetArrayElement(i));
            }
        }
    };
}