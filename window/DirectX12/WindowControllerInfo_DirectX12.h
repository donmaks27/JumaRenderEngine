// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "renderEngine/window/WindowControllerInfo.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#if defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)
#include "WindowController_DirectX12_GLFW.h"
namespace JumaRenderEngine
{
    template<>
    struct WindowControllerInfo<RenderAPI::DirectX12> : std::true_type
    {
        static WindowController* create() { return new WindowController_DirectX12_GLFW(); }
    };
}
#endif

#endif