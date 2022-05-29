// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/VertexBuffer.h"

namespace JumaRenderEngine
{
    class VulkanBuffer;

    class VertexBuffer_Vulkan : public VertexBuffer
    {
    public:
        VertexBuffer_Vulkan() = default;
        virtual ~VertexBuffer_Vulkan() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(const VertexBufferData* verticesData) override;

    private:

        VulkanBuffer* m_VertexBuffer = nullptr;
        VulkanBuffer* m_IndexBuffer = nullptr;

        uint32 m_RenderElementsCount = 0;


        void clearVulkan();
    };
}

#endif
