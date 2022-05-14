// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    class RenderTarget;

    struct RenderOptions
    {
        RenderTarget* renderTarget = nullptr;
    };
}
