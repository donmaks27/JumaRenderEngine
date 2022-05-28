// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderTarget.h"

#include "jutils/jarray.h"
#include "vulkanObjects/VulkanFramebufferData.h"

namespace JumaRenderEngine
{
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

        VulkanRenderPass* m_RenderPass = nullptr;
        jarray<VulkanFramebufferData> m_Framebuffers;


        bool initFramebuffer();
        bool initWindowFramebuffer();

        void clearVulkan();

        int32 getRequiredFramebufferIndex() const;
    };
}

#endif
