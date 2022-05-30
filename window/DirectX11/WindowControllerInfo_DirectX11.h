// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "renderEngine/window/WindowControllerInfo.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#if defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)
#include "WindowController_DirectX11_GLFW.h"
namespace JumaRenderEngine
{
    template<>
    struct WindowControllerInfo<RenderAPI::DirectX11> : std::true_type
    {
        static WindowController* create() { return new WindowController_DirectX11_GLFW(); }
    };
}
#endif

#endif