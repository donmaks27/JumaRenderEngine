// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VulkanRenderPass.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    VulkanRenderPass::~VulkanRenderPass()
    {
        clearVulkan();
    }

    bool VulkanRenderPass::init(const VulkanRenderPassDescription& description, const render_pass_type_id renderPassTypeID)
    {
        const bool resolveEnabled = description.sampleCount != VK_SAMPLE_COUNT_1_BIT;
        const int32 attachmentCount = resolveEnabled ? (description.shouldUseDepth ? 3 : 2) : (description.shouldUseDepth ? 2 : 1);

        VkAttachmentDescription attachments[3];
        VkAttachmentReference attachmentRefs[3];

        VkAttachmentDescription& colorAttachment = attachments[0];
        colorAttachment.flags = 0;
        colorAttachment.format = description.colorFormat;
        colorAttachment.samples = description.sampleCount;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = (description.renderToSwapchain && !resolveEnabled) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference& colorAttachmentRef = attachmentRefs[0];
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        if (description.shouldUseDepth)
        {
            VkAttachmentDescription& depthAttachment = attachments[1];
            depthAttachment.flags = 0;
            depthAttachment.format = description.depthFormat;
            depthAttachment.samples = description.sampleCount;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            VkAttachmentReference& depthAttachmentRef = attachmentRefs[1];
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        if (resolveEnabled)
        {
            const int32 index = attachmentCount - 1;
            VkAttachmentDescription& colorResolveAttachment = attachments[index];
            colorResolveAttachment.flags = 0;
            colorResolveAttachment.format = description.colorFormat;
            colorResolveAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorResolveAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorResolveAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorResolveAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorResolveAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorResolveAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorResolveAttachment.finalLayout = description.renderToSwapchain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            VkAttachmentReference& colorResolveAttachmentRef = attachmentRefs[index];
            colorResolveAttachmentRef.attachment = index;
            colorResolveAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &attachmentRefs[0];
        subpass.pDepthStencilAttachment = description.shouldUseDepth ? &attachmentRefs[1] : nullptr;
        subpass.pResolveAttachments = resolveEnabled ? &attachmentRefs[2] : nullptr;
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        if (description.shouldUseDepth)
        {
            dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachmentCount;
        renderPassInfo.pAttachments = attachments;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        const VkResult result = vkCreateRenderPass(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), &renderPassInfo, nullptr, &m_RenderPass);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create render pass"));
            return false;
        }

        m_Description = description;
        m_RenderPassTypeID = renderPassTypeID;
        return true;
    }

    void VulkanRenderPass::clearVulkan()
    {
        if (m_RenderPass != nullptr)
        {
            vkDestroyRenderPass(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), m_RenderPass, nullptr);
            m_RenderPass = nullptr;
        }
        m_RenderPassTypeID = render_pass_type_id_INVALID;
    }
}

#endif
