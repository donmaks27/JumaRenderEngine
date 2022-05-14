// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include <vulkan/vulkan_core.h>

#include "jutils/juid.h"

namespace JumaRenderEngine
{
    using render_pass_type_id = uint32;
    constexpr render_pass_type_id render_pass_type_id_INVALID = juid<render_pass_type_id>::invalidUID;

    struct VulkanRenderPassDescription
    {
        VkFormat colorFormat = VK_FORMAT_UNDEFINED;
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

        bool shouldUseDepth = true;
        bool renderToSwapchain = true;

        struct compatible_predicate
        {
            constexpr bool operator()(const VulkanRenderPassDescription& description1, const VulkanRenderPassDescription& description2) const;
        };
        struct equal_predicate : compatible_predicate
        {
            constexpr bool operator()(const VulkanRenderPassDescription& description1, const VulkanRenderPassDescription& description2) const;
        };
    };

    constexpr bool VulkanRenderPassDescription::compatible_predicate::operator()(const VulkanRenderPassDescription& description1, 
        const VulkanRenderPassDescription& description2) const
    {
        if (description1.colorFormat != description2.colorFormat)
        {
            return description1.colorFormat < description2.colorFormat;
        }
        if (description1.shouldUseDepth != description2.shouldUseDepth)
        {
            return description2.shouldUseDepth;
        }
        if (description1.shouldUseDepth)
        {
            if (description1.depthFormat != description2.depthFormat)
            {
                return description1.depthFormat < description2.depthFormat;
            }
        }
        return description1.sampleCount < description2.sampleCount;
    }
    constexpr bool VulkanRenderPassDescription::equal_predicate::operator()(const VulkanRenderPassDescription& description1, 
        const VulkanRenderPassDescription& description2) const
    {
        return compatible_predicate::operator()(description1, description2) || (!description1.renderToSwapchain && description2.renderToSwapchain);
    }
}

#endif
