// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngineContextObject.h"

#include <vma/vk_mem_alloc.h>

#include "VulkanQueueType.h"
#include "jutils/math/vector2.h"
#include "renderEngine/texture/TextureFormat.h"
#include "renderEngine/texture/TextureSamples.h"

namespace JumaRenderEngine
{
    constexpr VkFormat GetVulkanFormatByTextureFormat(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA8: return VK_FORMAT_R8G8B8A8_SRGB;
        case TextureFormat::BGRA8: return VK_FORMAT_B8G8R8A8_SRGB;
        case TextureFormat::DEPTH32: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::DEPTH24_STENCIL8: return VK_FORMAT_D24_UNORM_S8_UINT;
        default: ;
        }
        return VK_FORMAT_UNDEFINED;
    }
    constexpr VkSampleCountFlagBits GetVulkanSampleCountByTextureSamples(const TextureSamples samples)
    {
        switch (samples)
        {
        case TextureSamples::X2: return VK_SAMPLE_COUNT_2_BIT;
        case TextureSamples::X4: return VK_SAMPLE_COUNT_4_BIT;
        case TextureSamples::X8: return VK_SAMPLE_COUNT_8_BIT;
        case TextureSamples::X16: return VK_SAMPLE_COUNT_16_BIT;
        case TextureSamples::X32: return VK_SAMPLE_COUNT_32_BIT;
        case TextureSamples::X64: return VK_SAMPLE_COUNT_64_BIT;
        default: ;
        }
        return VK_SAMPLE_COUNT_1_BIT;
    }

    class VulkanImage : public RenderEngineContextObject
    {
    public:
        VulkanImage() = default;
        virtual ~VulkanImage() override;

        bool init(VkImageUsageFlags usage, std::initializer_list<VulkanQueueType> accessedQueues, const math::uvector2& size, 
            VkSampleCountFlagBits sampleCount, VkFormat format, uint32 mipLevels);
        bool init(VkImageUsageFlags usage, std::initializer_list<VulkanQueueType> accessedQueues, const math::uvector2& size, 
            VkSampleCountFlagBits sampleCount, VkFormat format);
        bool init(VkImage existingImage, const math::uvector2& size, VkFormat format, uint32 mipLevels);

        bool createImageView(VkImageAspectFlags aspectFlags);

        VkImage get() const { return m_Image; }
        VkImageView getImageView() const { return m_ImageView; }

        void changeImageLayout(VkCommandBuffer commandBuffer,
            VkImageLayout oldLayout, VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
            VkImageLayout newLayout, VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
        void copyImage(VkCommandBuffer commandBuffer, 
            const VulkanImage* srcImage, uint32 srcMipLevel, uint32 dstMipLevel,
            VkImageLayout newLayout, VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);
        void generateMipmaps(VkCommandBuffer commandBuffer, VkImageLayout newLayout, VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

        bool setImageData(const uint8* data, 
            VkImageLayout oldLayout, VkAccessFlags srcAccess, VkPipelineStageFlags srcStage,
            VkImageLayout newLayout, VkAccessFlags dstAccess, VkPipelineStageFlags dstStage);

    protected:

        virtual void clearInternal() override { clearVulkan(); }

    private:

        VkImage m_Image = nullptr;
        VmaAllocation m_Allocation = nullptr;
        VkImageView m_ImageView = nullptr;

        math::uvector2 m_Size = { 0, 0 };
        VkFormat m_Format = VK_FORMAT_UNDEFINED;
        uint32 m_MipLevels = 0;


        void clearVulkan();
    };
}

#endif
