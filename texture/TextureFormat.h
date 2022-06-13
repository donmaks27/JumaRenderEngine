// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    enum class TextureFormat : uint8
    {
        RGBA_UINT8,
        BGRA_UINT8,
        DEPTH_FLOAT32,
        DEPTH_UNORM24_STENCIL_UINT8
    };

    constexpr uint32 GetTextureFormatSize(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA_UINT8:
        case TextureFormat::BGRA_UINT8:
        case TextureFormat::DEPTH_FLOAT32:
        case TextureFormat::DEPTH_UNORM24_STENCIL_UINT8:
            return 4;

        default: ;
        }
        return 0;
    }
}
