#pragma once
#include <cstdint>

namespace Online::Audio
{
    enum class AudioQueue : uint16_t
    {
        BackgroundMusic = 1000,  // 背景音乐（唯一通道，永不被抢占）
        WorldSFX = 2000,         // 世界环境音效（可被抢占）
        UISFX = 4000,            // UI交互音效（高优先级）
        CriticalSFX = 5000       // 关键游戏音效（最高优先级）
    };
}
