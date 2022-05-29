// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VulkanImage.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "VulkanCommandPool.h"
#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"

namespace JumaRenderEngine
{
    constexpr uint32 GetTextureFormatSizeByVulkanFormat(const VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_R8G8B8A8_SRGB: return GetTextureFormatSize(TextureFormat::RGBA_UINT8);
        case VK_FORMAT_B8G8R8A8_SRGB: return GetTextureFormatSize(TextureFormat::BGRA_UINT8);
        case VK_FORMAT_D32_SFLOAT: return GetTextureFormatSize(TextureFormat::DEPTH_FLOAT32);
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return GetTextureFormatSize(TextureFormat::DEPTH_FLOAT32_STENCIL_UINT8);
        case VK_FORMAT_D24_UNORM_S8_UINT: return GetTextureFormatSize(TextureFormat::DEPTH_UNORM24_STENCIL_UINT8);
        default: ;
        }
        return 0;
    }

    VulkanImage::~VulkanImage()
    {
        clearVulkan();
    }

    void VulkanImage::clearVulkan()
    {
        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        if (m_ImageView != nullptr)
        {
            vkDestroyImageView(renderEngine->getDevice(), m_ImageView, nullptr);
            m_ImageView = nullptr;
        }
        if (m_Allocation != nullptr)
        {
            vmaDestroyImage(renderEngine->getAllocator(), m_Image, m_Allocation);
            m_Allocation = nullptr;
        }
        m_Image = nullptr;

        m_Size = { 0, 0 };
        m_Format = VK_FORMAT_UNDEFINED;
    }

    bool VulkanImage::init(const VkImageUsageFlags usage, const std::initializer_list<VulkanQueueType> accessedQueues,
        const math::uvector2& size, const VkSampleCountFlagBits sampleCount, const VkFormat format, const uint32 mipLevels)
    {
        if (isValid())
        {
            JUMA_RENDER_LOG(warning, JSTR("Vulkan image already initialized"));
            return false;
        }

        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        jarray<uint32> accessedQueueFamilies;
        accessedQueueFamilies.reserve(static_cast<int32>(accessedQueues.size()));
        for (const auto& queue : accessedQueues)
        {
            accessedQueueFamilies.addUnique(renderEngine->getQueue(queue)->familyIndex);
        }

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = size.x;
        imageInfo.extent.height = size.y;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        if (accessedQueueFamilies.getSize() > 1)
        {
            imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            imageInfo.queueFamilyIndexCount = static_cast<uint32>(accessedQueueFamilies.getSize());
            imageInfo.pQueueFamilyIndices = accessedQueueFamilies.getData();
        }
        else
        {
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.queueFamilyIndexCount = 0;
            imageInfo.pQueueFamilyIndices = nullptr;
        }
        imageInfo.samples = sampleCount;
        imageInfo.flags = 0;
        VmaAllocationCreateInfo allocationInfo{};
        allocationInfo.flags = 0;
        allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocationInfo.preferredFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        const VkResult result = vmaCreateImage(renderEngine->getAllocator(), &imageInfo, &allocationInfo, &m_Image, &m_Allocation, nullptr);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan image"));
            return false;
        }

        m_Size = size;
        m_Format = format;
        m_MipLevels = mipLevels;
        m_ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        markAsInitialized();
        return true;
    }
    bool VulkanImage::init(const VkImageUsageFlags usage, const std::initializer_list<VulkanQueueType> accessedQueues,
        const math::uvector2& size, const VkSampleCountFlagBits sampleCount, const VkFormat format)
    {
        //return init(usage, accessedQueues, size, sampleCount, format, static_cast<uint32>(std::floor(std::log2(math::min(size.x, size.y)))) + 1);
        return init(usage, accessedQueues, size, sampleCount, format, 1);
    }
    bool VulkanImage::init(VkImage existingImage, const math::uvector2& size, const VkFormat format, const uint32 mipLevels)
    {
        if (isValid())
        {
            JUMA_RENDER_LOG(warning, JSTR("Vulkan image already initialized"));
            return false;
        }
        if ((existingImage == nullptr) || (size.x == 0) || (size.y == 0) || (mipLevels == 0))
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid input params"));
            return false;
        }

        m_Image = existingImage;
        m_Allocation = nullptr;
        m_Size = size;
        m_Format = format;
        m_MipLevels = mipLevels;
        markAsInitialized();
        return true;
    }

    bool VulkanImage::createImageView(const VkImageAspectFlags aspectFlags)
    {
        if (!isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Vulkan image not initialized"));
            return false;
        }
        if (m_ImageView != nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Vulkan image view already created"));
            return false;
        }

        VkImageViewCreateInfo imageViewInfo{};
	    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	    imageViewInfo.image = m_Image;
	    imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	    imageViewInfo.format = m_Format;
	    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	    imageViewInfo.subresourceRange.aspectMask = aspectFlags;
	    imageViewInfo.subresourceRange.baseMipLevel = 0;
	    imageViewInfo.subresourceRange.levelCount = m_MipLevels;
	    imageViewInfo.subresourceRange.baseArrayLayer = 0;
	    imageViewInfo.subresourceRange.layerCount = 1;
        const VkResult result = vkCreateImageView(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), &imageViewInfo, nullptr, &m_ImageView);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan image view"));
            return false;
        }
        return true;
    }

    bool VulkanImage::changeImageLayout(VkCommandBuffer commandBuffer, const VkImageLayout newLayout, const VkAccessFlags srcAccess,
        const VkPipelineStageFlags srcStage, const VkAccessFlags dstAccess, const VkPipelineStageFlags dstStage)
    {
        if (!isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Vulkan image not initialized"));
            return false;
        }
        if (commandBuffer == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid vulkan command buffer"));
            return false;
        }

        VkImageMemoryBarrier imageBarrier{};
        imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrier.oldLayout = m_ImageLayout;
        imageBarrier.newLayout = newLayout;
        imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageBarrier.image = m_Image;
        imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrier.subresourceRange.baseMipLevel = 0;
        imageBarrier.subresourceRange.levelCount = m_MipLevels;
        imageBarrier.subresourceRange.baseArrayLayer = 0;
        imageBarrier.subresourceRange.layerCount = 1;
        imageBarrier.srcAccessMask = srcAccess;
        imageBarrier.dstAccessMask = dstAccess;
        vkCmdPipelineBarrier(
            commandBuffer, srcStage, dstStage, 0, 
            0, nullptr, 0, nullptr, 1, &imageBarrier
        );

        m_ImageLayout = newLayout;
        return true;
    }
    bool VulkanImage::changeImageLayout(VkCommandBuffer commandBuffer, const VkImageLayout newLayout)
    {
        if ((m_ImageLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
        {
            return changeImageLayout(commandBuffer, newLayout, 
                0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT
            );
        }
        if ((m_ImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
        {
            return changeImageLayout(commandBuffer, newLayout, 
                VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            );
        }

        JUMA_RENDER_LOG(warning, JSTR("Vulkan image transition not implemented"));
        return false;
    }

    bool VulkanImage::setImageData(const uint8* data, const VkImageLayout finalLayout)
    {
        if (!isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Vulkan image not initialized"));
            return false;
        }
        if (data == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid input params"));
            return false;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        VulkanCommandBuffer* commandBuffer = renderEngine->getCommandPool(VulkanQueueType::Graphics)->getCommandBuffer();
        if (commandBuffer == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get vulkan command buffer"));
            return false;
        }

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer->get(), &beginInfo);

        if (m_ImageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            if (!changeImageLayout(commandBuffer->get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to change vulkan image layout"));
                commandBuffer->returnToCommandPool();
                return false;
            }
        }

        const uint32 imageSize = m_Size.x * m_Size.y * GetTextureFormatSizeByVulkanFormat(m_Format);
        VulkanBuffer* stagingBuffer = renderEngine->getVulkanBuffer();
        if (!stagingBuffer->initStaging(imageSize) || !stagingBuffer->setData(data, imageSize, 0, true))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create staging buffer"));
            commandBuffer->returnToCommandPool();
            renderEngine->returnVulkanBuffer(stagingBuffer);
            return false;
        }

        VkBufferImageCopy imageCopy{};
        imageCopy.bufferOffset = 0;
        imageCopy.bufferRowLength = 0;
        imageCopy.bufferImageHeight = 0;
        imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageCopy.imageSubresource.mipLevel = 0;
        imageCopy.imageSubresource.baseArrayLayer = 0;
        imageCopy.imageSubresource.layerCount = 1;
        imageCopy.imageOffset = { 0, 0, 0 };
        imageCopy.imageExtent = { m_Size.x, m_Size.y, 1 };
        vkCmdCopyBufferToImage(commandBuffer->get(), stagingBuffer->get(), m_Image, m_ImageLayout, 1, &imageCopy);

        if (m_ImageLayout != finalLayout)
        {
            if (!changeImageLayout(commandBuffer->get(), finalLayout))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to change vulkan image layout"));
                commandBuffer->returnToCommandPool();
                renderEngine->returnVulkanBuffer(stagingBuffer);
                return false;
            }
        }

        vkEndCommandBuffer(commandBuffer->get());
        if (!commandBuffer->submit(true))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to submit render command buffer"));
            commandBuffer->returnToCommandPool();
            renderEngine->returnVulkanBuffer(stagingBuffer);
            return false;
        }

        commandBuffer->returnToCommandPool();
        renderEngine->returnVulkanBuffer(stagingBuffer);
        return true;
    }
}

#endif
