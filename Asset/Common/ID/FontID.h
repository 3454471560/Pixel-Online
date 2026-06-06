#pragma once
#include <Asset/Common/ID/TextureID.h>
#include <cstdint>

namespace Online::Asset
{
    enum class FontID : uint8_t
    {
        Invalid,
        Ipix,
        Deng,
        Count

    };

    inline TextureID FontIDToTextureID(FontID ID)
    {
        TextureID id = TextureID::Tex_FontIpixAtlas;
        switch (ID)
        {
        case FontID::Ipix:
            id = TextureID::Tex_FontIpixAtlas;
            break;
		case FontID::Deng:
            id = TextureID::Tex_FontDengAtlas;
			break;
        default:
            id = TextureID::Tex_FontIpixAtlas;
            break;
        }
        return id;
    }

}
