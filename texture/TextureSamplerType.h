// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    enum class TextureFiltering : uint8
    {
        Point, Bilinear, Trilinear, Anisotropic_2, Anisotropic_4, Anisotropic_8, Anisotropic_16
    };
    enum class TextureWrapMode : uint8
    {
        Repeat, Mirror, Clamp
    };
    struct TextureSamplerType
    {
        TextureFiltering filtering = TextureFiltering::Point;
        TextureWrapMode wrapMode = TextureWrapMode::Clamp;

        bool operator<(const TextureSamplerType& type) const { return (filtering < type.filtering) || ((filtering == type.filtering) && (wrapMode < type.wrapMode)); }
    };
}
