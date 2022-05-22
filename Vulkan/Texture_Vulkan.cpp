// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Texture_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderEngine_Vulkan.h"
#include "vulkanObjects/VulkanCommandPool.h"
#include "vulkanObjects/VulkanImage.h"

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

        VulkanImage* image = renderEngine->createObject<VulkanImage>();
        const bool imageInitialized = image->init(
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, 
            size, TextureSamples::X1, format
        );
        if (!imageInitialized)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize vulkan image"));
            commandBuffer->returnToCommandPool();
            delete image;
            return false;
        }

        if (!image->setImageData(commandBuffer->get(), data) || 
            !image->generateMipmaps(commandBuffer->get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to load data to vulkan image"));
            commandBuffer->returnToCommandPool();
            delete image;
            return false;
        }
        if (!image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vukan image view"));
            commandBuffer->returnToCommandPool();
            delete image;
            return false;
        }

        m_Image = image;
        return true;
    }

    void Texture_Vulkan::clearVulkan()
    {
        if (m_Image != nullptr)
        {
            delete m_Image;
            m_Image = nullptr;
        }
    }
}

#endif
