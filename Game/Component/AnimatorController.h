#pragma once
#include <Game/Component/Component.h>
#include <Game/Component/Sprite.h>
#include <Game/Component/Animator.h>
#include <Asset/Common/ID/AnimationClipID.h>
#include <Asset/Common/Data/AnimationClip.h>
#include <Asset/Common/FuncTable.h>
#include <Log/Common/FuncTable.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <variant>

namespace Online::Game
{
    enum class AnimatorParameterType : uint8_t
    {
        Float,
        Bool,
        Trigger
    };

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

    enum class AnimatorConditionMode : uint8_t
    {
        If,
        IfNot,
        Greater,
        Less,
    };

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

    struct AnimatorController : public Component
    {
    public:
        void Serialize(Online::Serialize::SerializeContext& ctx) const override
        {
            ctx.Write("version", 1);

            ctx.BeginArray("parameters");
            for (const auto& [name, param] : parameters)
            {
                Serialize::SerializeContext& pCtx = ctx.WriteArrayObjectBegin();
                param.Serialize(pCtx);
            }
            ctx.EndArray();

            ctx.BeginArray("states");
            for (const auto& state : states)
            {
                Serialize::SerializeContext& sCtx = ctx.WriteArrayObjectBegin();
                state.Serialize(sCtx);
            }
            ctx.EndArray();

            ctx.BeginArray("transitions");
            for (const auto& trans : transitions)
            {
                Serialize::SerializeContext& tCtx = ctx.WriteArrayObjectBegin();
                trans.Serialize(tCtx);
            }
            ctx.EndArray();

            ctx.Write("defaultState", defaultStateName);
            ctx.Write("currentState", currentStateName);
        }

        void Deserialize(const Online::Serialize::DeserializeContext& ctx) override
        {
            int version = 0;
            ctx.Read("version", version);

            const auto& paramsCtx = ctx.GetSubContext("parameters");
            size_t pCount = 0;
            if (paramsCtx.GetArraySize("", pCount))
            {
                parameters.clear();
                for (size_t i = 0; i < pCount; ++i)
                {
                    AnimatorParameter p;
                    p.Deserialize(paramsCtx.GetArrayElement(i));
                    parameters[p.Name] = p;
                }
            }

            const auto& statesCtx = ctx.GetSubContext("states");
            size_t sCount = 0;
            if (statesCtx.GetArraySize("", sCount))
            {
                states.resize(sCount);
                for (size_t i = 0; i < sCount; ++i)
                    states[i].Deserialize(statesCtx.GetArrayElement(i));
            }

            const auto& transCtx = ctx.GetSubContext("transitions");
            size_t tCount = 0;
            if (transCtx.GetArraySize("", tCount))
            {
                transitions.resize(tCount);
                for (size_t i = 0; i < tCount; ++i)
                    transitions[i].Deserialize(transCtx.GetArrayElement(i));
            }

            ctx.Read("defaultState", defaultStateName);
            ctx.Read("currentState", currentStateName);

        }

        void SetMainAnimator(Animator* anim) noexcept
        {
            mainAnimator = anim;
            mainSprite = anim ? anim->GetSprite() : nullptr;
            if (mainAnimator && currentStateName.empty())
                Play(defaultStateName);
        }

        void SetOverlayAnimator(Animator* anim) noexcept
        {
            overlayAnimator = anim;
            overlaySprite = anim ? anim->GetSprite() : nullptr;
            if (overlaySprite)
                overlaySprite->OnDisable(); // Ä¬ÈÏÒþ²Ø
        }

        void SetFloat(const std::string& name, float value)
        {
            auto it = parameters.find(name);
            if (it != parameters.end())
            {
                it->second.FloatValue = value;
                it->second.Type = AnimatorParameterType::Float;
            }
            else
            {
                parameters[name] = { name, AnimatorParameterType::Float, value };
            }
        }
        void SetBool(const std::string& name, bool value)
        {
            SetFloat(name, value ? 1.0f : 0.0f);
            parameters[name].Type = AnimatorParameterType::Bool;
        }
        void SetTrigger(const std::string& name)
        {
            SetFloat(name, 1.0f);
            parameters[name].Type = AnimatorParameterType::Trigger;
        }

        float GetFloat(const std::string& name) const
        {
            auto it = parameters.find(name);
            return (it != parameters.end()) ? it->second.FloatValue : 0.0f;
        }
        bool GetBool(const std::string& name) const
        {
            return GetFloat(name) != 0.0f;
        }

        void Play(const std::string& stateName)
        {
            if (!mainAnimator || !mainSprite) return;
            if (currentStateName == stateName && !isCrossfading) return;

            auto it = std::find_if(states.begin(), states.end(),
                [&](const AnimatorState& s) { return s.Name == stateName; });
            if (it == states.end())
            {
                Log::Error("AnimatorController: State not found - " + stateName);
                return;
            }

            if (isCrossfading)
                EndCrossfade();

            currentStateName = stateName;

            mainAnimator->Play(it->ClipID);
            mainSprite->SetAlpha(1.0f);

            if (overlaySprite)
                overlaySprite->OnDisable();
        }

        void Crossfade(const std::string& stateName, float duration)
        {
            if (!mainAnimator || !overlayAnimator || !mainSprite || !overlaySprite) return;
            if (currentStateName == stateName || isCrossfading) return;

            auto it = std::find_if(states.begin(), states.end(),
                [&](const AnimatorState& s) { return s.Name == stateName; });
            if (it == states.end()) return;

            targetStateName = stateName;
            crossfadeDuration = duration;
            crossfadeTimer = 0.0f;
            isCrossfading = true;

            overlayAnimator->Play(it->ClipID);
            overlaySprite->SetAlpha(0.0f);
            overlaySprite->OnEnable();
        }

        void EndCrossfade()
        {
            isCrossfading = false;

            auto it = std::find_if(states.begin(), states.end(),
                [&](const AnimatorState& s) { return s.Name == targetStateName; });
            if (it != states.end() && mainAnimator)
            {
                currentStateName = targetStateName;
                mainAnimator->Play(it->ClipID);
                mainSprite->SetAlpha(1.0f);
            }

            if (overlaySprite)
            {
                overlaySprite->OnDisable();
                overlaySprite->SetAlpha(1.0f);
            }
            crossfadeTimer = 0.0f;
        }

        inline std::unordered_map<std::string, AnimatorParameter>& GetParameters() noexcept { return parameters; }
        inline void SetParameters(const std::unordered_map<std::string, AnimatorParameter>& inParams) noexcept { parameters = inParams; }
        inline const AnimatorParameter* GetParameter(const std::string& paramName) const noexcept
        {
            auto it = parameters.find(paramName);
            return it != parameters.end() ? &it->second : nullptr;
        }
        inline AnimatorParameterType GetParameterType(const std::string& paramName) const noexcept
        {
            auto p = GetParameter(paramName);
            return p ? p->Type : AnimatorParameterType::Float;
        }

        inline const std::vector<AnimatorState>& GetStates() const noexcept 
        { 
            return states;
        }
        inline void SetStates(const std::vector<AnimatorState>& inStates) noexcept 
        { 
            states = inStates; 
        }
        inline const AnimatorState* GetState(const std::string& stateName) const noexcept
        {
            auto it = std::find_if(states.cbegin(), states.cend(), [&](const AnimatorState& s) { return s.Name == stateName; });
            return it != states.cend() ? &(*it) : nullptr;
        }

        inline const std::vector<AnimatorTransition>& GetTransitions() const noexcept 
        { 
            return transitions;
        }
        inline void SetTransitions(const std::vector<AnimatorTransition>& inTrans) noexcept 
        { 
            transitions = inTrans; 
        }

        inline const std::string& GetDefaultStateName() const noexcept 
        { 
            return defaultStateName;
        }
        inline void SetDefaultStateName(const std::string& name) noexcept 
        { 
            defaultStateName = name; 
        }

        inline const std::string& GetCurrentStateName() const noexcept 
        { 
            return currentStateName; 
        }
        inline void SetCurrentStateName(const std::string& name) noexcept
        { 
            currentStateName = name; 
        }

        inline Sprite* GetMainSprite() const noexcept 
        { 
            return mainSprite; 
        }
        inline Sprite* GetOverlaySprite() const noexcept 
        { 
            return overlaySprite; 
        }

        inline Animator* GetMainAnimator() const noexcept 
        {
            return mainAnimator; 
        }
        inline Animator* GetOverlayAnimator() const noexcept 
        { 
            return overlayAnimator; 
        }

        inline bool GetIsCrossfading() const noexcept 
        { 
            return isCrossfading; 
        }
        inline void SetIsCrossfading(bool b) noexcept 
        { 
            isCrossfading = b; 
        }

        inline const std::string& GetTargetStateName() const noexcept 
        { 
            targetStateName; 
        }
        inline void SetTargetStateName(const std::string& name) noexcept 
        { 
            targetStateName = name; 
        }

        inline float GetCrossfadeDuration() const noexcept 
        { 
            return crossfadeDuration;
        }
        inline void SetCrossfadeDuration(float val) noexcept 
        { 
            crossfadeDuration = val;
        }

        inline float GetCrossfadeTimer() noexcept 
        { 
            return crossfadeTimer;
        }
        inline void SetCrossfadeTimer(float val) noexcept
        { 
            crossfadeTimer = val; 
        }

    private:
        std::vector<AnimatorState> states;
        std::vector<AnimatorTransition> transitions;
        std::unordered_map<std::string, AnimatorParameter> parameters;
        std::string defaultStateName = "Idle";
        std::string currentStateName;

        Animator* mainAnimator = nullptr;
        Animator* overlayAnimator = nullptr;

        Sprite* mainSprite = nullptr;
        Sprite* overlaySprite = nullptr;

        bool isCrossfading = false;
        std::string targetStateName;
        float crossfadeDuration = 0.0f;
        float crossfadeTimer = 0.0f;
    };
}