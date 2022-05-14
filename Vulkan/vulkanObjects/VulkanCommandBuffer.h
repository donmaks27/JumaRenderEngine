// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngineContextObject.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    class VulkanCommandPool;

    class VulkanCommandBuffer
    {
        friend VulkanCommandPool;

    public:
        VulkanCommandBuffer() = default;
        ~VulkanCommandBuffer() = default;

        VkCommandBuffer get() const { return m_CommandBuffer; }

        bool submit(bool waitForFinish);
        bool submit(VkSubmitInfo submitInfo, VkFence fenceOnFinish, bool waitForFinish);

        void returnToCommandPool();

    private:

        VulkanCommandPool* m_CommandPool = nullptr;
        VkCommandBuffer m_CommandBuffer = nullptr;
    };
}

#endif
