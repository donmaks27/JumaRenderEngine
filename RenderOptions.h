// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    class RenderPipeline;
    class RenderTarget;

    struct RenderOptions
    {
        RenderPipeline* renderPipeline = nullptr;
        RenderTarget* renderTarget = nullptr;
    };
}
