// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderTarget.h"

#include <vulkan/vulkan_core.h>

#include "jutils/jarray.h"
#include "vulkanObjects/VulkanRenderPassDescription.h"

namespace JumaRenderEngine
{
    class VulkanImage;
    class VulkanRenderPass;

    class RenderTarget_Vulkan : public RenderTarget
    {
        using Super = RenderTarget;

    public:
        RenderTarget_Vulkan() = default;
        virtual ~RenderTarget_Vulkan() override;

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    protected:
        
        virtual bool initInternal() override;

    private:

        struct FramebufferData
        {
            VkFramebuffer framebuffer = nullptr;
            VulkanImage* colorAttachment = nullptr;
            VulkanImage* depthAttachment = nullptr;
            VulkanImage* resolveAttachment = nullptr;
        };

        VulkanRenderPass* m_RenderPass = nullptr;
        jarray<FramebufferData> m_Framebuffers;


        bool createFramebuffers();
        bool createWindowFramebuffers();

        bool createFramebuffer(const VulkanRenderPassDescription& renderPassDescription, VkImage resultImage, FramebufferData& outFramebuffer) const;

        void clearVulkan();

        int32 getRequiredFramebufferIndex() const;
    };
}

#endif
