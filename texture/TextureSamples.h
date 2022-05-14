// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    enum class TextureSamples : uint8
    {
        X1, X2, X4, X8, X16, X32, X64
    };

    inline uint8 GetTextureSamplesNumber(const TextureSamples samples)
    {
        switch (samples)
        {
        case TextureSamples::X1:  return 1;
        case TextureSamples::X2:  return 2;
        case TextureSamples::X4:  return 4;
        case TextureSamples::X8:  return 8;
        case TextureSamples::X16: return 16;
        case TextureSamples::X32: return 32;
        case TextureSamples::X64: return 64;
        default: ;
        }
        JUMA_RENDER_LOG(error, JSTR("Unsupported sample count"));
        return 0;
    }
}
