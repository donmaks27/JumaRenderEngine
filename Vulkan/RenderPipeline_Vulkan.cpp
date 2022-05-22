// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderPipeline_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderOptions_Vulkan.h"

namespace JumaRenderEngine
{
    RenderPipeline_Vulkan::~RenderPipeline_Vulkan()
    {
        clearVulkan();
    }

    void RenderPipeline_Vulkan::clearVulkan()
    {
    }

    void RenderPipeline_Vulkan::renderInternal()
    {
        callRender<RenderOptions_Vulkan>();
    }
    bool RenderPipeline_Vulkan::onStartRender()
    {
        return Super::onStartRender();
    }
    void RenderPipeline_Vulkan::onFinishRender()
    {
        Super::onFinishRender();
    }
}

#endif
