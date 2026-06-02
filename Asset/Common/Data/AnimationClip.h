#pragma once
#include <Core/String/String.h>
#include <Asset/Common/ID/TextureID.h>

#include <glm.hpp>

#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>

namespace Online::Asset
{
    struct AnimationClip
    {
    public:
        AnimationClip() = default;
        AnimationClip(const std::string& name,int x,int y,int rate,bool loop,const std::vector<int>& frames)
            :GridCols(x), GridRows(y), FrameRate(static_cast<float>(rate)),Looping(loop)
        {
            SetAnimationName(name);
            SetAnimationTextureID(name);
            SetAnimationFrames(frames);
        }
    public:
        inline const uint8_t* GetFrames() const noexcept
        {
            return Frames;
        }
        inline uint8_t GetFrameCount() const noexcept
        {
            return FrameCount;
        }
        inline float GetFrameRate() const noexcept
        {
            return FrameRate;
        }
        inline bool IsLooping() const noexcept
        {
            return Looping;
        }
        inline float GetDuration() const noexcept
        {
            return FrameCount <= 0 ? 0.0f : static_cast<float>(FrameCount) / FrameRate;
        }
        inline TextureID GetTextureID() const noexcept
        {
            return TextureID;
        }
        inline uint8_t GetGridCols() const noexcept
        {
            return GridCols;
        }
        inline uint8_t GetGridRows() const noexcept
        {
            return GridRows;
        }
        inline const char* GetName() const noexcept
        {
            return Name;
        }

    public:
        inline void SetFrameRate(float fps) noexcept
        {
            FrameRate = fps > 0.0001f ? fps : 10.0f;
        }
        inline void SetLooping(bool looping) noexcept
        {
            Looping = looping;
        }
        inline void SetTextureID(TextureID id) noexcept
        {
            TextureID = id;
        }
        inline void SetGrid(uint8_t cols, uint8_t rows) noexcept
        {
            GridCols = cols > 1 ? cols : 1;
            GridRows = rows > 1 ? rows : 1;
        }
        inline void SetName(const char* name) noexcept
        {
            strncpy_s(Name, name, MAX_NAME_LENGTH - 1);
            Name[MAX_NAME_LENGTH - 1] = '\0';
        }
        inline bool AddFrame(uint8_t frameIndex) noexcept
        {
            if (FrameCount >= MAX_ANIMATION_FRAMES) { return false; }
            Frames[FrameCount++] = frameIndex;
            return true;
        }
        inline void SetFrames(const uint8_t* frames, uint8_t count) noexcept
        {
            FrameCount = count >= MAX_ANIMATION_FRAMES ? MAX_ANIMATION_FRAMES - 1 : count;
            memcpy(Frames, frames, FrameCount * sizeof(uint8_t));
        }
        inline uint8_t GetFrameAt(uint8_t index) const noexcept
        {
            if (index >= FrameCount) { return 0; }
            return Frames[index];
        }

    private:
        void SetAnimationName(const std::string& baseName)
        {
            std::memset(Name, 0, MAX_NAME_LENGTH);

            constexpr const char* ANIM_PREFIX = "Anim_";
            constexpr size_t PREFIX_LEN = 5;

            const size_t maxBaseNameLen = MAX_NAME_LENGTH - PREFIX_LEN - 1;
            const size_t baseNameLen = baseName.length();
            const size_t copyLen = (baseNameLen < maxBaseNameLen) ? baseNameLen : maxBaseNameLen;

            std::memcpy(Name, ANIM_PREFIX, PREFIX_LEN);
            std::memcpy(Name + PREFIX_LEN, baseName.c_str(), copyLen);
        }

        void SetAnimationFrames(const std::vector<int>& frames)
        {
            std::memset(Frames, 0, MAX_ANIMATION_FRAMES);

            const size_t safeCount = (frames.size() < static_cast<size_t>(MAX_ANIMATION_FRAMES))
                ? frames.size()
                : static_cast<size_t>(MAX_ANIMATION_FRAMES);
            FrameCount = static_cast<uint8_t>(safeCount);

            for (size_t i = 0; i < safeCount; ++i)
            {
                int val = frames[i];
                val = std::clamp(val, 0, 255);
                Frames[i] = static_cast<uint8_t>(val);
            }
        }

        void SetAnimationTextureID(const std::string& name)
        {
            TextureID = StringToTextureID(Online::Core::Prepend("Tex_", name));
        }

    public:
        inline static const constexpr uint8_t MAX_ANIMATION_FRAMES = 64;
        inline static const constexpr uint8_t MAX_NAME_LENGTH = 32;

    private:
        uint8_t Frames[MAX_ANIMATION_FRAMES] = { 0 };
        uint8_t FrameCount = 0;
        float FrameRate = 10.0f;
        bool Looping = true;
        TextureID TextureID = TextureID::Tex_Default;
        uint8_t GridCols = 1;
        uint8_t GridRows = 1;
        char Name[MAX_NAME_LENGTH] = { 0 };
    };
}