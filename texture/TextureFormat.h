// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    enum class TextureFormat : uint8
    {
        RGB_UINT8,
        RGBA_UINT8,
        BGRA_UINT8,
        DEPTH_FLOAT32,
        DEPTH_FLOAT32_STENCIL_UINT8,
        DEPTH_UNORM24_STENCIL_UINT8
    };

    constexpr uint32 GetTextureFormatSize(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGB_UINT8:
            return 3;

        case TextureFormat::RGBA_UINT8:
        case TextureFormat::BGRA_UINT8:
        case TextureFormat::DEPTH_FLOAT32:
        case TextureFormat::DEPTH_UNORM24_STENCIL_UINT8:
            return 4;

        case TextureFormat::DEPTH_FLOAT32_STENCIL_UINT8: 
            return 5;

        default: ;
        }
        return 0;
    }
}
