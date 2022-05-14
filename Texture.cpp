// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Texture.h"

namespace JumaRenderEngine
{
    bool Texture::init(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        if ((size.x == 0) || (size.y == 0) || (data == nullptr))
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid nput params"));
            return false;
        }

        if (!initInternal(size, format, data))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize texture"));
            return false;
        }
        return true;
    }
}
