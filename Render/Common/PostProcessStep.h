#pragma once

namespace Online::Render
{
    enum class PostProcessStep
    {
        None,           // 无效果
        Alpha,      // 灰度
        BlendMode,    // 反色
        ScaleMode,           // 模糊
        Color,     // 亮度调整
        ToneMapping     // 色调映射
    };
}