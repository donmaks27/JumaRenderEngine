// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "Material_Vulkan.h"
#include "RenderEngine_Vulkan.h"
#include "RenderOptions_Vulkan.h"
#include "renderEngine/Material.h"
#include "renderEngine/vertex/VertexBufferData.h"
#include "vulkanObjects/VulkanBuffer.h"
#include "vulkanObjects/VulkanCommandBuffer.h"

namespace JumaRenderEngine
{
    VertexBuffer_Vulkan::~VertexBuffer_Vulkan()
    {
        clearVulkan();
    }

    bool VertexBuffer_Vulkan::initInternal(const VertexBufferData* verticesData)
    {
        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();

        const uint32 vertexCount = verticesData->getVertexCount();
        if (vertexCount == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("Empty vertex buffer data"));
            return false;
        }
        const VertexDescription* vertexDescription = renderEngine->findVertexType(getVertexTypeName());

        VulkanBuffer* vertexBuffer = renderEngine->createObject<VulkanBuffer>();
        bool success = vertexBuffer->initGPU(
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, 
            vertexDescription->size * vertexCount, verticesData->getVertices(), true
        );
        if (!success)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize vulkan vertex buffer"));
            delete vertexBuffer;
            return false;
        }

        const uint32 indexCount = verticesData->getIndexCount();
        if (indexCount > 0)
        {
            VulkanBuffer* indexBuffer = renderEngine->createObject<VulkanBuffer>();
            success = indexBuffer->initGPU(
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, { VulkanQueueType::Graphics, VulkanQueueType::Transfer }, 
                sizeof(uint32) * vertexCount, verticesData->getIndices(), true
            );
            if (!success)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to initialize vulkan index buffer"));
                delete indexBuffer;
                delete vertexBuffer;
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
        if (m_IndexBuffer != nullptr)
        {
            delete m_IndexBuffer;
            m_IndexBuffer = nullptr;
        }
        if (m_VertexBuffer != nullptr)
        {
            delete m_VertexBuffer;
            m_VertexBuffer = nullptr;
        }
        m_RenderElementsCount = 0;
    }

    void VertexBuffer_Vulkan::render(const RenderOptions* renderOptions, Material* material)
    {
        if ((renderOptions == nullptr) || (material == nullptr))
        {
            return;
        }

        Material_Vulkan* materialVulan = dynamic_cast<Material_Vulkan*>(material);
        if (!materialVulan->bindMaterial(renderOptions, this))
        {
            return;
        }

        const RenderOptions_Vulkan* optionsVulkan = reinterpret_cast<const RenderOptions_Vulkan*>(renderOptions);
        VkCommandBuffer commandBuffer = optionsVulkan->commandBuffer->get();

        VkBuffer vertexBuffer = m_VertexBuffer->get();
        const VkDeviceSize offset = 0;
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
