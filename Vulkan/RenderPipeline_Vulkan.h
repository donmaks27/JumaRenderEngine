// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderPipeline.h"

namespace JumaRenderEngine
{
    class RenderPipeline_Vulkan : public RenderPipeline
    {
        using Super = RenderPipeline;

    public:
        RenderPipeline_Vulkan() = default;
        virtual ~RenderPipeline_Vulkan() override;

    protected:

        virtual void renderInternal() override;
        virtual bool onStartRender() override;
        virtual void onFinishRender() override;

    private:

        void clearVulkan();
    };
}

#endif
