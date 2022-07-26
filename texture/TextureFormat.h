// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    enum class TextureFormat : uint8
    {
        RGBA8,
        BGRA8,
        DEPTH32,
        DEPTH24_STENCIL8
    };

    constexpr uint32 GetTextureFormatSize(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8:
        case TextureFormat::BGRA8:
        case TextureFormat::DEPTH32:
        case TextureFormat::DEPTH24_STENCIL8:
            return 4;

        default: ;
        }
        return 0;
    }
}
