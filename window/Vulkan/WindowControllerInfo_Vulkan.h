// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "renderEngine/window/WindowControllerInfo.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#if defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)
#include "WindowController_Vulkan_GLFW.h"
namespace JumaRenderEngine
{
    template<>
    struct WindowControllerInfo<RenderAPI::Vulkan> : std::true_type
    {
        static WindowController* create() { return new WindowController_Vulkan_GLFW(); }
    };
}
#endif

#endif
