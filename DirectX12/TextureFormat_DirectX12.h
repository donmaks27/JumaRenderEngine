﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/texture/TextureFormat.h"

#include <dxgiformat.h>

namespace JumaRenderEngine
{
    constexpr DXGI_FORMAT GetDirectX12FormatByTextureFormat(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA_UINT8: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::BGRA_UINT8: return DXGI_FORMAT_B8G8R8A8_UNORM;
        case TextureFormat::DEPTH_FLOAT32: return DXGI_FORMAT_D32_FLOAT;
        case TextureFormat::DEPTH_UNORM24_STENCIL_UINT8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        default: ;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
}

#endif
