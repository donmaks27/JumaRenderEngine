// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VulkanCommandBuffer.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "VulkanCommandPool.h"
#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    bool VulkanCommandBuffer::submit(const bool waitForFinish)
    {
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        return submit(submitInfo, nullptr, waitForFinish);
    }
    bool VulkanCommandBuffer::submit(VkSubmitInfo submitInfo, VkFence fenceOnFinish, const bool waitForFinish)
    {
        const VulkanQueueDescription* queueDescription = m_CommandPool->getRenderEngine<RenderEngine_Vulkan>()->getQueue(m_CommandPool->getQueueType());
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffer;
        const VkResult result = vkQueueSubmit(queueDescription->queue, 1, &submitInfo, fenceOnFinish);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to submit command buffer"));
            return false;
        }

        if (waitForFinish)
        {
            vkQueueWaitIdle(queueDescription->queue);
        }
        return true;
    }

    void VulkanCommandBuffer::returnToCommandPool()
    {
        m_CommandPool->returnCommandBuffer(this);
    }
}

#endif
