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

        VulkanImage* image = renderEngine->getVulkanImage();
        const bool imageInitialized = image->init(
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
            { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, size, VK_SAMPLE_COUNT_1_BIT, GetVulkanFormatByTextureFormat(format)
        );
        if (!imageInitialized)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize vulkan image"));
            renderEngine->returnVulkanImage(image);
            return false;
        }

        const bool setImageDataSuccess = image->setImageData(data, 
            VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
        if (!setImageDataSuccess)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to load data to vulkan image"));
            renderEngine->returnVulkanImage(image);
            return false;
        }
        if (!image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vukan image view"));
            renderEngine->returnVulkanImage(image);
            return false;
        }

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
