// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngineContextObject.h"

#include "VulkanRenderPassDescription.h"

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

    private:

        VkRenderPass m_RenderPass = nullptr;

        VulkanRenderPassDescription m_Description;
        render_pass_type_id m_RenderPassTypeID = render_pass_type_id_INVALID;


        bool init(const VulkanRenderPassDescription& description, render_pass_type_id renderPassTypeID);

        void clearVulkan();
    };
}

#endif
