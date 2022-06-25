// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/RenderPipeline.h"

namespace JumaRenderEngine
{
    class RenderPipeline_DirectX11 : public RenderPipeline
    {
    public:
        RenderPipeline_DirectX11() = default;
        virtual ~RenderPipeline_DirectX11() override = default;

    protected:

        virtual math::vector2 getScreenCoordsModifier() const override { return { 1.0f, -1.0f }; }
    };
}

#endif
