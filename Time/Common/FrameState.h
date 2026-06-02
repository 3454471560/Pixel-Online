#pragma once

#include<cstdint>

namespace Online::Time
{
    struct FrameState
    {
        float delta = 0.0f;
        float fixdelta = 1.0f / 60.0f;
        float unscaledDeltaTime = 0.0f;
        float scale = 1.0f;
        float FPS = 60.0f;
        float targetFPS = 60.0f;
        double seconds = 0.0;
        int64_t milliseconds = 0;
    };
}
