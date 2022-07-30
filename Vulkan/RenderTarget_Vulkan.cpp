// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderTarget_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderEngine_Vulkan.h"
#include "RenderOptions_Vulkan.h"
#include "renderEngine/window/Vulkan/WindowController_Vulkan.h"
#include "vulkanObjects/VulkanCommandBuffer.h"
#include "vulkanObjects/VulkanSwapchain.h"

namespace JumaRenderEngine
{
    RenderTarget_Vulkan::~RenderTarget_Vulkan()
    {
        clearVulkan();
    }

    bool RenderTarget_Vulkan::initInternal()
    {
        if (!Super::initInternal())
        {
            return false;
        }
        if (!isWindowRenderTarget() ? !initRenderTarget() : !initWindowRenderTarget())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan framebuffers"));
            clearVulkan();
            return false;
        }
        return true;
    }
    bool RenderTarget_Vulkan::initRenderTarget()
    {
        VulkanRenderPassDescription renderPassDescription;
        renderPassDescription.colorFormat = GetVulkanFormatByTextureFormat(getFormat());
        renderPassDescription.depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        renderPassDescription.sampleCount = GetVulkanSampleCountByTextureSamples(getSampleCount());
        renderPassDescription.shouldUseDepth = true;
        renderPassDescription.renderToSwapchain = false;
        m_RenderPass = getRenderEngine<RenderEngine_Vulkan>()->getRenderPass(renderPassDescription);
        if (m_RenderPass == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get vulkan render pass"));
            return false;
        }

        VulkanFramebufferData framebufferData;
        if (!m_RenderPass->createVulkanFramebuffer(getSize(), framebufferData))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan framebuffer"));
            return false;
        }
        m_Framebuffers = { framebufferData };
        return true;
    }
    bool RenderTarget_Vulkan::initWindowRenderTarget()
    {
        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const WindowData_Vulkan* windowData = renderEngine->getWindowController()->findWindowData<WindowData_Vulkan>(getWindowID());
        VulkanSwapchain* swapchain = windowData != nullptr ? windowData->vulkanSwapchain : nullptr;
        if (swapchain == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to find swapchain for window ") + TO_JSTR(getWindowID()));
            return false;
        }
        if (!createWindowFramebuffers(swapchain))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create window frambuffers"));
            return false;
        }

        swapchain->OnParentWindowPropertiesChanged.bind(this, &RenderTarget_Vulkan::onWindowPropertiesChanged);
        return true;
    }
    bool RenderTarget_Vulkan::createWindowFramebuffers(const VulkanSwapchain* swapchain)
    {
        if (swapchain == nullptr)
        {
            const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
            const WindowData_Vulkan* windowData = renderEngine->getWindowController()->findWindowData<WindowData_Vulkan>(getWindowID());
            swapchain = windowData != nullptr ? windowData->vulkanSwapchain : nullptr;
            if (swapchain == nullptr)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to find swapchain for window ") + TO_JSTR(getWindowID()));
                return false;
            }
        }

        VulkanRenderPassDescription renderPassDescription;
        renderPassDescription.colorFormat = swapchain->getImagesFormat();
        renderPassDescription.depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        renderPassDescription.sampleCount = GetVulkanSampleCountByTextureSamples(getSampleCount());
        renderPassDescription.shouldUseDepth = true;
        renderPassDescription.renderToSwapchain = true;
        m_RenderPass = getRenderEngine<RenderEngine_Vulkan>()->getRenderPass(renderPassDescription);
        if (m_RenderPass == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get vulkan render pass"));
            return false;
        }

        const math::uvector2& size = swapchain->getImagesSize();
        const jarray<VkImage>& swapchainImages = swapchain->getImages();
        m_Framebuffers.resize(swapchainImages.getSize());
        for (int32 index = 0; index < swapchainImages.getSize(); index++)
        {
            if (!m_RenderPass->createVulkanSwapchainFramebuffer(size, swapchainImages[index], m_Framebuffers[index]))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan framebuffer for swapchain image ") + TO_JSTR(index));
                return false;
            }
        }
        return true;
    }

    void RenderTarget_Vulkan::clearVulkan()
    {
        if (isWindowRenderTarget())
        {
            const WindowData_Vulkan* windowData = getRenderEngine()->getWindowController()->findWindowData<WindowData_Vulkan>(getWindowID());
            if ((windowData != nullptr) && (windowData->vulkanSwapchain != nullptr))
            {
                windowData->vulkanSwapchain->OnParentWindowPropertiesChanged.unbind(this, &RenderTarget_Vulkan::onWindowPropertiesChanged);
            }
        }

        clearFramebuffers();
    }
    void RenderTarget_Vulkan::clearFramebuffers()
    {
        if (!m_Framebuffers.isEmpty())
        {
            RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
            VkDevice device = renderEngine->getDevice();
            for (const auto& framebuffer : m_Framebuffers)
            {
                if (framebuffer.framebuffer != nullptr)
                {
                    vkDestroyFramebuffer(device, framebuffer.framebuffer, nullptr);
                }
                renderEngine->returnVulkanImage(framebuffer.colorAttachment);
                renderEngine->returnVulkanImage(framebuffer.depthAttachment);
                renderEngine->returnVulkanImage(framebuffer.resolveAttachment);
                renderEngine->returnVulkanImage(framebuffer.resultImage);
            }
            m_Framebuffers.clear();
        }
        m_RenderPass = nullptr;
        m_FramebuffersValidForRender = false;
    }

    VulkanImage* RenderTarget_Vulkan::getResultImage() const
    {
        if (!isWindowRenderTarget() && !m_Framebuffers.isEmpty())
        {
            return m_Framebuffers[0].resultImage;
        }
        return nullptr;
    }

    void RenderTarget_Vulkan::onWindowPropertiesChanged(VulkanSwapchain* swapchain)
    {
        const math::uvector2& size = swapchain->getImagesSize();
        const TextureSamples samples = getRenderEngine()->getWindowController()->findWindowData(getWindowID())->properties.samples;
        changeProperties(size, samples);
    }
    void RenderTarget_Vulkan::onPropertiesChanged(const math::uvector2& prevSize, TextureSamples prevSamples)
    {
        Super::onPropertiesChanged(prevSize, prevSamples);

        clearFramebuffers();
        createWindowFramebuffers();
    }

    bool RenderTarget_Vulkan::onStartRender(RenderOptions* renderOptions)
    {
        const int32 framebufferIndex = getRequiredFramebufferIndex();
        if (!m_Framebuffers.isValidIndex(framebufferIndex))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get vulkan framebuffer"));
            return false;
        }

        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }

        RenderOptions_Vulkan* renderOptionsVulkan = reinterpret_cast<RenderOptions_Vulkan*>(renderOptions);
        renderOptionsVulkan->renderPass = m_RenderPass;

        const VulkanFramebufferData& framebuffer = m_Framebuffers[framebufferIndex];
        VkCommandBuffer commandBuffer = renderOptionsVulkan->commandBuffer->get();
        if (!m_FramebuffersValidForRender)
        {
            if (!isWindowRenderTarget())
            {
                VulkanImage* resutAttachmentImage = framebuffer.resolveAttachment != nullptr ? framebuffer.resolveAttachment : framebuffer.colorAttachment;
                resutAttachmentImage->changeImageLayout(commandBuffer,
                    VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
                framebuffer.resultImage->changeImageLayout(commandBuffer, 
                    VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            }
            m_FramebuffersValidForRender = true;
        }

        const math::uvector2 size = getSize();
        VkClearValue clearValues[2];
        clearValues[0].color = { { 1.0f, 1.0f, 1.0f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass->get();
        renderPassInfo.framebuffer = framebuffer.framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { size.x, size.y };
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(size.x);
        viewport.height = static_cast<float>(size.y);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor;
        scissor.offset = { 0, 0 };
        scissor.extent = { size.x, size.y };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        return true;
    }
    void RenderTarget_Vulkan::onFinishRender(RenderOptions* renderOptions)
    {
        VkCommandBuffer commandBuffer = reinterpret_cast<RenderOptions_Vulkan*>(renderOptions)->commandBuffer->get();
        vkCmdEndRenderPass(commandBuffer);

        if (!isWindowRenderTarget())
        {
            const VulkanFramebufferData& framebuffer = m_Framebuffers[0];
            VulkanImage* resultAttachment = framebuffer.resolveAttachment != nullptr ? framebuffer.resolveAttachment : framebuffer.colorAttachment;

            resultAttachment->changeImageLayout(commandBuffer,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
            );
            framebuffer.resultImage->changeImageLayout(commandBuffer,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
            );
            framebuffer.resultImage->copyImage(commandBuffer, resultAttachment, 0, 0,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            resultAttachment->changeImageLayout(commandBuffer,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            );
        }

        Super::onFinishRender(renderOptions);
    }

    int32 RenderTarget_Vulkan::getRequiredFramebufferIndex() const
    {
        if (m_Framebuffers.isEmpty())
        {
            return -1;
        }
        if (!isWindowRenderTarget())
        {
            return 0;
        }
        const WindowData_Vulkan* windowData = getRenderEngine()->getWindowController()->findWindowData<WindowData_Vulkan>(getWindowID());
        return windowData->vulkanSwapchain->getAcquiredImageIndex();
    }
}

#endif
