// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    enum class TextureFilterType : uint8
    {
        Point, Bilinear, Trilinear, Anisotropic_2, Anisotropic_4, Anisotropic_8, Anisotropic_16
    };
    enum class TextureWrapMode : uint8
    {
        Repeat, Mirror, Clamp
    };

    constexpr uint8 TextureFilterTypeCount = 7;
    constexpr uint8 TextureWrapModeCount = 3;
    constexpr uint8 TextureSamplerTypeCount = TextureFilterTypeCount * TextureWrapModeCount;

    struct TextureSamplerType
    {
        TextureFilterType filterType = TextureFilterType::Point;
        TextureWrapMode wrapMode = TextureWrapMode::Clamp;

        bool operator<(const TextureSamplerType& type) const { return (filterType < type.filterType) || ((filterType == type.filterType) && (wrapMode < type.wrapMode)); }
    };

    constexpr uint8 GetTextureSamplerTypeID(const TextureSamplerType& samplerType)
    {
        return static_cast<uint8>(static_cast<uint8>(samplerType.filterType) * TextureWrapModeCount + static_cast<uint8>(samplerType.wrapMode));
    }
    constexpr TextureSamplerType GetTextureSamplerType(const uint8 samplerTypeID)
    {
        return {
            static_cast<TextureFilterType>(samplerTypeID / TextureWrapModeCount),
            static_cast<TextureWrapMode>(samplerTypeID % TextureWrapModeCount)
        };
    }
}
