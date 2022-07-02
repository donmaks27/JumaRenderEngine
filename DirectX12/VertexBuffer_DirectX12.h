// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/VertexBuffer.h"

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX12 : public VertexBuffer
    {
    public:
        VertexBuffer_DirectX12() = default;
        virtual ~VertexBuffer_DirectX12() override = default;
    };
}

#endif
