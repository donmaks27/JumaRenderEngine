// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderTarget.h"

namespace JumaRenderEngine
{
    class RenderTarget_DirectX12 : public RenderTarget
    {
    public:
        RenderTarget_DirectX12() = default;
        virtual ~RenderTarget_DirectX12() override = default;
    };
}

#endif
