// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Texture_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderEngine_Vulkan.h"
#include "vulkanObjects/VulkanCommandPool.h"

namespace JumaRenderEngine
{
    Texture_Vulkan::~Texture_Vulkan()
    {
        clearVulkan();
    }

    bool Texture_Vulkan::initInternal(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        VulkanCommandBuffer* commandBuffer = renderEngine->getCommandPool(VulkanQueueType::Graphics)->getCommandBuffer();
        if (commandBuffer == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get vulkan command buffer"));
            return false;
        }

        VulkanImage* image = renderEngine->getVulkanImage();
        const bool imageInitialized = image->init(
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
            { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, size, VK_SAMPLE_COUNT_1_BIT, GetVulkanFormatByTextureFormat(format)
        );
        if (!imageInitialized)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize vulkan image"));
            commandBuffer->returnToCommandPool();
            renderEngine->returnVulkanImage(image);
            return false;
        }

        if (!image->setImageData(data, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to load data to vulkan image"));
            commandBuffer->returnToCommandPool();
            renderEngine->returnVulkanImage(image);
            return false;
        }
        if (!image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vukan image view"));
            commandBuffer->returnToCommandPool();
            renderEngine->returnVulkanImage(image);
            return false;
        }

        /*VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(commandBuffer->get(), &beginInfo);
        if (!image->generateMipmaps(commandBuffer->get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to generate mipmaps for vulkan image"));
            commandBuffer->returnToCommandPool();
            delete image;
            return false;
        }
        vkEndCommandBuffer(commandBuffer->get());
        if (!commandBuffer->submit(true))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to submit vulkan command buffer"));
            commandBuffer->returnToCommandPool();
            return false;
        }*/

        commandBuffer->returnToCommandPool();
        m_Image = image;
        return true;
    }

    void Texture_Vulkan::clearVulkan()
    {
        if (m_Image != nullptr)
        {
            getRenderEngine<RenderEngine_Vulkan>()->returnVulkanImage(m_Image);
            m_Image = nullptr;
        }
    }
}

#endif
