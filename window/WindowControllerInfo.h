// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "renderEngine/RenderAPI.h"
#include "WindowController.h"

namespace JumaRenderEngine
{
    template<RenderAPI API>
    struct WindowControllerInfo : std::false_type
    {
        static WindowController* create()
        {
            JUMA_RENDER_LOG(error, JSTR("Unsupported render API"));
            return nullptr;
        }
    };
}
