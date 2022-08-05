// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VulkanCommandPool.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    VulkanCommandPool::~VulkanCommandPool()
    {
        clearVulkan();
    }

    bool VulkanCommandPool::init(const VulkanQueueType queueType, const VkCommandPoolCreateFlags flags)
    {
        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const VulkanQueueDescription* queueDescription = renderEngine->getQueue(queueType);
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.queueFamilyIndex = queueDescription->familyIndex;
        commandPoolInfo.flags = flags | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        const VkResult result = vkCreateCommandPool(renderEngine->getDevice(), &commandPoolInfo, nullptr, &m_CommandPool);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan command pool for queue family {}"), queueDescription->familyIndex);
            return false;
        }

        m_QueueType = queueType;
        return true;
    }

    void VulkanCommandPool::clearVulkan()
    {
        m_UnusedCommandBuffers.clear();
        m_CommandBuffers.clear();
        if (m_CommandPool != nullptr)
        {
            vkDestroyCommandPool(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), m_CommandPool, nullptr);
            m_CommandPool = nullptr;
        }
    }

    VulkanCommandBuffer* VulkanCommandPool::getCommandBuffer()
    {
        if (!m_UnusedCommandBuffers.isEmpty())
        {
            VulkanCommandBuffer* result = m_UnusedCommandBuffers.getLast();
            m_UnusedCommandBuffers.removeLast();
            return result;
        }

        VulkanCommandBuffer& commandBuffer = m_CommandBuffers.addDefault();
        if (!createCommandBuffer(true, commandBuffer))
        {
            m_CommandBuffers.removeLast();
            return nullptr;
        }

        commandBuffer.m_CommandPool = this;
        return &commandBuffer;
    }
    bool VulkanCommandPool::createCommandBuffer(const bool primaryLevel, VulkanCommandBuffer& outCommandBuffers)
    {
        VkCommandBufferAllocateInfo commandBufferInfo{};
        commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferInfo.commandPool = m_CommandPool;
        commandBufferInfo.level = primaryLevel ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        commandBufferInfo.commandBufferCount = 1;
        const VkResult result = vkAllocateCommandBuffers(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), &commandBufferInfo, &outCommandBuffers.m_CommandBuffer);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to allocate command buffer"));
            return false;
        }
        return true;
    }

    void VulkanCommandPool::returnCommandBuffer(VulkanCommandBuffer* commandBuffer)
    {
        if (commandBuffer != nullptr)
        {
            vkResetCommandBuffer(commandBuffer->get(), VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
            m_UnusedCommandBuffers.add(commandBuffer);
        }
    }
}

#endif
