// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "Material_Vulkan.h"
#include "RenderEngine_Vulkan.h"
#include "RenderOptions_Vulkan.h"
#include "renderEngine/vertex/VertexBufferData.h"
#include "vulkanObjects/VulkanCommandBuffer.h"

namespace JumaRenderEngine
{
    VertexBuffer_Vulkan::~VertexBuffer_Vulkan()
    {
        clearVulkan();
    }

    bool VertexBuffer_Vulkan::initInternal(VertexBufferData* verticesData)
    {
        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        const uint32 vertexCount = verticesData->getVertexCount();
        if (vertexCount == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("Empty vertex buffer data"));
            return false;
        }
        const VertexDescription* vertexDescription = renderEngine->findVertexType(getVertexTypeName());

        VulkanBuffer* vertexBuffer = renderEngine->getVulkanBuffer();
        vertexBuffer->initGPU(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, 
            vertexDescription->size * vertexCount, verticesData->getVertices()
        );
        if (!vertexBuffer->isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize vulkan vertex buffer"));
            renderEngine->returnVulkanBuffer(vertexBuffer);
            return false;
        }

        const uint32 indexCount = verticesData->getIndexCount();
        if (indexCount > 0)
        {
            VulkanBuffer* indexBuffer = renderEngine->getVulkanBuffer();
            indexBuffer->initGPU(
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, 
                sizeof(uint32) * vertexCount, verticesData->getIndices()
            );
            if (!indexBuffer->isValid())
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to initialize vulkan index buffer"));
                renderEngine->returnVulkanBuffer(vertexBuffer);
                renderEngine->returnVulkanBuffer(indexBuffer);
                return false;
            }

            m_IndexBuffer = indexBuffer;
            m_RenderElementsCount = indexCount;
        }
        else
        {
            m_RenderElementsCount = vertexCount;
        }

        m_VertexBuffer = vertexBuffer;
        return true;
    }

    void VertexBuffer_Vulkan::clearVulkan()
    {
        if (m_VertexBuffer != nullptr)
        {
            RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
            renderEngine->returnVulkanBuffer(m_VertexBuffer);
            renderEngine->returnVulkanBuffer(m_IndexBuffer);

            m_VertexBuffer = nullptr;
            m_IndexBuffer = nullptr;
            m_RenderElementsCount = 0;
        }
    }

    void VertexBuffer_Vulkan::render(const RenderOptions* renderOptions, Material* material)
    {
        Material_Vulkan* materialVulan = dynamic_cast<Material_Vulkan*>(material);
        if (!materialVulan->bindMaterial(renderOptions, this))
        {
            return;
        }

        const RenderOptions_Vulkan* optionsVulkan = reinterpret_cast<const RenderOptions_Vulkan*>(renderOptions);
        VkCommandBuffer commandBuffer = optionsVulkan->commandBuffer->get();

        VkBuffer vertexBuffer = m_VertexBuffer->get();
        constexpr VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &offset);
        if (m_IndexBuffer == nullptr)
        {
            vkCmdDraw(commandBuffer, m_RenderElementsCount, 1, 0, 0);
        }
        else
        {
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->get(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, m_RenderElementsCount, 1, 0, 0, 0);
        }

        materialVulan->unbindMaterial(renderOptions, this);
    }
}

#endif
