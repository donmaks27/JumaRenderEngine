// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderOptions.h"

namespace JumaRenderEngine
{
    class DirectX12CommandList;

    struct RenderOptions_DirectX12 : RenderOptions
    {
        DirectX12CommandList* renderCommandList = nullptr;
    };
}

#endif
