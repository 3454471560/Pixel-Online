#pragma once
#include<Core/Color/Color.h>
#include<Asset/Common/ID/TextureID.h>

#include <glm.hpp>

namespace Online::Render
{
    struct RenderTargetSnapshot
    {
        Asset::TextureID TargetTexture = Asset::TextureID::Tex_BackBuffer_1;
        glm::ivec2 Offset = { 0, 0 };
        glm::ivec2 Size = { 0, 0 };
        glm::vec4 ClearColor = Online::Core::Color::SkyBlue;
        bool ClearEnable = true;

        RenderTargetSnapshot(Asset::TextureID textureID, glm::ivec2 offset, glm::ivec2 size, Online::Core::Color Clear)
			: TargetTexture(textureID), Offset(offset), Size(size), ClearColor(Clear)
        {
        }

        RenderTargetSnapshot(Asset::TextureID textureID, glm::ivec2 offset, glm::ivec2 size, bool ClearEnable)
			: TargetTexture(textureID), Offset(offset), Size(size), ClearEnable(ClearEnable)
        {
        }

        RenderTargetSnapshot(Asset::TextureID textureID, glm::ivec2 offset, glm::ivec2 size)
            : TargetTexture(textureID),Offset(offset), Size(size)
        {
        }

        RenderTargetSnapshot(glm::ivec2 offset, glm::ivec2 size)
            : Offset(offset), Size(size)
        {
        }

        RenderTargetSnapshot() = default;

    };
}

