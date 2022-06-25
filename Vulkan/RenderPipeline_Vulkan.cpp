// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderPipeline_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderEngine_Vulkan.h"
#include "RenderOptions_Vulkan.h"
#include "renderEngine/RenderTarget.h"
#include "renderEngine/window/Vulkan/WindowController_Vulkan.h"
#include "vulkanObjects/VulkanCommandBuffer.h"
#include "vulkanObjects/VulkanCommandPool.h"
#include "vulkanObjects/VulkanSwapchain.h"

namespace JumaRenderEngine
{
    RenderPipeline_Vulkan::~RenderPipeline_Vulkan()
    {
        clearVulkan();
    }

    bool RenderPipeline_Vulkan::initInternal()
    {
        if (!Super::initInternal())
        {
            return false;
        }

        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkResult result = vkCreateFence(device, &fenceInfo, nullptr, &m_RenderFinishedFence);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan fence"));
            return false;
        }

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan semaphore"));
            clearVulkan();
            return false;
        }

        return true;
    }

    void RenderPipeline_Vulkan::clearVulkan()
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        m_SwapchainImageReadySemaphores.clear();
        m_Swapchains.clear();
        if (m_RenderCommandBuffer != nullptr)
        {
            m_RenderCommandBuffer->returnToCommandPool();
            m_RenderCommandBuffer = nullptr;
        }
        if (m_RenderFinishedSemaphore != nullptr)
        {
            vkDestroySemaphore(device, m_RenderFinishedSemaphore, nullptr);
            m_RenderFinishedSemaphore = nullptr;
        }
        if (m_RenderFinishedFence != nullptr)
        {
            vkDestroyFence(device, m_RenderFinishedFence, nullptr);
            m_RenderFinishedFence = nullptr;
        }
    }

    void RenderPipeline_Vulkan::renderInternal()
    {
        callRender<RenderOptions_Vulkan>();
    }

    bool RenderPipeline_Vulkan::onStartRender(RenderOptions* renderOptions)
    {
        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }

        // Wait for prev render frame finished
        waitForPreviousRenderFinish();

        // Acquire next swapchain images
        const WindowController* windowController = getRenderEngine()->getWindowController();
        m_Swapchains.clear();
        m_SwapchainImageReadySemaphores.clear();
        for (const auto& windowID : windowController->getWindowIDs())
        {
            const WindowData_Vulkan* windowData = windowController->findWindowData<WindowData_Vulkan>(windowID);
            VulkanSwapchain* swapchain = windowData != nullptr ? windowData->vulkanSwapchain : nullptr;
            if (swapchain == nullptr)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to get data for window ") + TO_JSTR(windowID));
                return false;
            }
            bool availableForRender = false;
            if (!swapchain->acquireNextImage(availableForRender) || !availableForRender)
            {
                return false;
            }

            m_Swapchains.add(swapchain);
            m_SwapchainImageReadySemaphores.add(swapchain->getRenderAvailableSemaphore());
        }

        return startRecordingRenderCommandBuffer(renderOptions);
    }
    void RenderPipeline_Vulkan::onFinishRender(RenderOptions* renderOptions)
    {
        finishRecordingRenderCommandBuffer(renderOptions);
        Super::onFinishRender(renderOptions);
    }
    void RenderPipeline_Vulkan::waitForRenderFinished()
    {
        waitForPreviousRenderFinish();
    }

    void RenderPipeline_Vulkan::waitForPreviousRenderFinish()
    {
        if (m_RenderCommandBuffer != nullptr)
        {
            vkWaitForFences(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), 1, &m_RenderFinishedFence, VK_TRUE, UINT64_MAX);
            m_RenderCommandBuffer->returnToCommandPool();
            m_RenderCommandBuffer = nullptr;
        }
    }
    bool RenderPipeline_Vulkan::startRecordingRenderCommandBuffer(RenderOptions* renderOptions)
    {
        VulkanCommandBuffer* commandBuffer = getRenderEngine<RenderEngine_Vulkan>()->getCommandPool(VulkanQueueType::Graphics)->getCommandBuffer();
        if (commandBuffer == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create render command buffer"));
            return false;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        const VkResult result = vkBeginCommandBuffer(commandBuffer->get(), &beginInfo);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to start render command buffer record"));
            commandBuffer->returnToCommandPool();
            return false;
        }

        reinterpret_cast<RenderOptions_Vulkan*>(renderOptions)->commandBuffer = commandBuffer;
        return true;
    }
    bool RenderPipeline_Vulkan::finishRecordingRenderCommandBuffer(RenderOptions* renderOptions)
    {
        VulkanCommandBuffer* commandBuffer = reinterpret_cast<RenderOptions_Vulkan*>(renderOptions)->commandBuffer;
        const VkResult result = vkEndCommandBuffer(commandBuffer->get());
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to finish render command buffer record"));
            commandBuffer->returnToCommandPool();
            return false;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        vkResetFences(renderEngine->getDevice(), 1, &m_RenderFinishedFence);

        const jarray<VkPipelineStageFlags> waitStages(m_SwapchainImageReadySemaphores.getSize(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = m_SwapchainImageReadySemaphores.getSize();
        submitInfo.pWaitSemaphores = m_SwapchainImageReadySemaphores.getData();
        submitInfo.pWaitDstStageMask = waitStages.getData();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RenderFinishedSemaphore;
        if (!commandBuffer->submit(submitInfo, m_RenderFinishedFence, false))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to submit vulkan render command buffer"));
            commandBuffer->returnToCommandPool();
            return false;
        }
        m_RenderCommandBuffer = commandBuffer;

        if (!m_Swapchains.isEmpty())
        {
            jarray<uint32> swapchainIndicesForPresent(m_Swapchains.getSize());
            jarray<VkSwapchainKHR> vulkanSwapchainsForPresent(m_Swapchains.getSize());
            jarray<VkResult> swapchainPresentResults(m_Swapchains.getSize());
            for (int32 index = 0; index < m_Swapchains.getSize(); index++)
            {
                vulkanSwapchainsForPresent[index] = m_Swapchains[index]->get();
                swapchainIndicesForPresent[index] = static_cast<uint8>(m_Swapchains[index]->getAcquiredImageIndex());
            }

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphore;
            presentInfo.swapchainCount = m_Swapchains.getSize();
            presentInfo.pSwapchains = vulkanSwapchainsForPresent.getData();
            presentInfo.pImageIndices = swapchainIndicesForPresent.getData();
            presentInfo.pResults = swapchainPresentResults.getData();
            vkQueuePresentKHR(renderEngine->getQueue(VulkanQueueType::Graphics)->queue, &presentInfo);

            for (int32 index = 0; index < m_Swapchains.getSize(); index++)
            {
                const VkResult presentResult = swapchainPresentResults[index];
                if ((presentResult == VK_ERROR_OUT_OF_DATE_KHR) || (presentResult == VK_SUBOPTIMAL_KHR))
                {
                    m_Swapchains[index]->markAsNeedToRecreate();
                }
                else if (presentResult != VK_SUCCESS)
                {
                    JUMA_RENDER_ERROR_LOG(presentResult, JSTR("Failed to present swapchain image"));
                    return false;
                }
            }
        }
        return true;
    }
}

#endif
