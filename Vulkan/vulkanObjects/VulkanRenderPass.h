// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngineContextObject.h"

#include "VulkanFramebufferData.h"
#include "VulkanRenderPassDescription.h"
#include "jutils/math/vector2.h"

namespace JumaRenderEngine
{
    class RenderEngine_Vulkan;

    class VulkanRenderPass : public RenderEngineContextObjectBase
    {
        friend RenderEngine_Vulkan;

    public:
        VulkanRenderPass() = default;
        virtual ~VulkanRenderPass() override;

        VkRenderPass get() const { return m_RenderPass; }

        const VulkanRenderPassDescription& getDescription() const { return m_Description; }
        render_pass_type_id getTypeID() const { return m_RenderPassTypeID; }

        bool createVulkanSwapchainFramebuffer(const math::uvector2& size, VkImage swapchainImage, VulkanFramebufferData& outFramebuffer) const;
        bool createVulkanFramebuffer(const math::uvector2& size, VulkanFramebufferData& outFramebuffer) const;

    private:

        VkRenderPass m_RenderPass = nullptr;

        VulkanRenderPassDescription m_Description;
        render_pass_type_id m_RenderPassTypeID = render_pass_type_id_INVALID;


        bool init(const VulkanRenderPassDescription& description, render_pass_type_id renderPassTypeID);

        void clearVulkan();

        bool createVulkanFramebufferInternal(const math::uvector2& size, VkImage resultImage, VulkanFramebufferData& outFramebuffer) const;
    };
}

#endif
