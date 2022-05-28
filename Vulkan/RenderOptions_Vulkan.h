// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderOptions.h"

namespace JumaRenderEngine
{
    class VulkanCommandBuffer;
    class VulkanRenderPass;

    struct RenderOptions_Vulkan : RenderOptions
    {
        const VulkanRenderPass* renderPass = nullptr;
        VulkanCommandBuffer* commandBuffer = nullptr;
    };
}

#endif
