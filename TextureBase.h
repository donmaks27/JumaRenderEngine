// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "renderEngine/RenderEngineContextObject.h"

#include "jutils/math/vector2.h"
#include "texture/TextureSamplerType.h"

namespace JumaRenderEngine
{
    inline int32 GetMipLevelCountByTextureSize(const math::uvector2& size)
    {
        return static_cast<uint32>(std::floor(std::log2(math::min(size.x, size.y)))) + 1;
    }

    class TextureBase : public RenderEngineContextObjectBase
    {
    public:
        TextureBase() = default;
        virtual ~TextureBase() override = default;

        TextureSamplerType getSamplerType() const { return m_Sampler; }
        void setSamplerType(const TextureSamplerType sampler) { m_Sampler = sampler; }

    private:

        TextureSamplerType m_Sampler = { TextureFilterType::Trilinear, TextureWrapMode::Clamp };
    };
}
