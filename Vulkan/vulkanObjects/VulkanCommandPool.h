// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngineContextObject.h"

#include <vulkan/vulkan_core.h>

#include "VulkanCommandBuffer.h"
#include "VulkanQueueType.h"
#include "jutils/jlist.h"

namespace JumaRenderEngine
{
    class RenderEngine_Vulkan;

    class VulkanCommandPool : public RenderEngineContextObjectBase
    {
        friend RenderEngine_Vulkan;

    public:
        VulkanCommandPool() = default;
        virtual ~VulkanCommandPool() override;

        VkCommandPool get() const { return m_CommandPool; }
        VulkanQueueType getQueueType() const { return m_QueueType; }

        VulkanCommandBuffer* getCommandBuffer();
        void returnCommandBuffer(VulkanCommandBuffer* commandBuffer);

    private:

        VkCommandPool m_CommandPool = nullptr;
        VulkanQueueType m_QueueType = VulkanQueueType::Graphics;

        jlist<VulkanCommandBuffer> m_CommandBuffers;
        jlist<VulkanCommandBuffer*> m_UnusedCommandBuffers;


        bool init(VulkanQueueType queueType, VkCommandPoolCreateFlags flags = 0);

        void clearVulkan();

        bool createCommandBuffer(bool primaryLevel, VulkanCommandBuffer& outCommandBuffers);
    };
}

#endif
