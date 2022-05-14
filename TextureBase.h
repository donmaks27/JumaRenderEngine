// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "renderEngine/RenderEngineContextObject.h"

#include "texture/TextureSamplerType.h"

namespace JumaRenderEngine
{
    class TextureBase : public RenderEngineContextObjectBase
    {
    public:
        TextureBase() = default;
        virtual ~TextureBase() override = default;

        TextureSamplerType getSamplerType() const { return m_Sampler; }
        void setSamplerType(const TextureSamplerType sampler) { m_Sampler = sampler; }

    private:

        TextureSamplerType m_Sampler = { TextureFiltering::Trilinear, TextureWrapMode::Clamp };
    };
}
