// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "renderEngine/window/WindowControllerInfo.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#if defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)
#include "WindowController_OpenGL_GLFW.h"
namespace JumaRenderEngine
{
    template<>
    struct WindowControllerInfo<RenderAPI::OpenGL> : std::true_type
    {
        static WindowController* create() { return new WindowController_OpenGL_GLFW(); }
    };
}
#endif

#endif