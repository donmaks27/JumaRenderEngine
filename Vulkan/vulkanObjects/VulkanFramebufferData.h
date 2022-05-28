// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    class VulkanImage;

    struct VulkanFramebufferData
    {
        VkFramebuffer framebuffer = nullptr;
        VulkanImage* colorAttachment = nullptr;
        VulkanImage* depthAttachment = nullptr;
        VulkanImage* resolveAttachment = nullptr;
    };
}

#endif
