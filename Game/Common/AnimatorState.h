#pragma once
#include <Serialize/Frontend/SerializeContext.h>
#include <Serialize/Frontend/DeserializeContext.h>
#include <Asset/Common/ID/AnimationClipID.h>
namespace Online::Game
{
    struct AnimatorState
    {
        std::string Name;
        Asset::AnimationClipID ClipID = Asset::AnimationClipID::Count;

        void Serialize(Online::Serialize::SerializeContext& ctx) const
        {
            ctx.Write("name", Name);
            ctx.Write("clipID", static_cast<int>(ClipID));
        }
        void Deserialize(const Online::Serialize::DeserializeContext& ctx)
        {
            ctx.Read("name", Name);
            int cid; ctx.Read("clipID", cid);
            ClipID = static_cast<Asset::AnimationClipID>(cid);
        }
    };
}