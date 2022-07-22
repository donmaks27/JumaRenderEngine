// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer_DirectX12.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "Material_DirectX12.h"
#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "renderEngine/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    VertexBuffer_DirectX12::~VertexBuffer_DirectX12()
    {
        clearDirectX();
    }

    bool VertexBuffer_DirectX12::initInternal(VertexBufferData* verticesData)
    {
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

        const uint32 vertexCount = verticesData->getVertexCount();
        if (vertexCount == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("Empty vertex buffer data"));
            return false;
        }
        const VertexDescription* vertexDescription = renderEngine->findVertexType(getVertexTypeName());

        DirectX12Buffer* vertexBuffer = renderEngine->getBuffer();
        if ((vertexBuffer == nullptr) || !vertexBuffer->initGPU(vertexDescription->size * vertexCount, verticesData->getVertices()))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vertex buffer"));
            renderEngine->returnBuffer(vertexBuffer);
            return false;
        }

        DirectX12Buffer* indexBuffer = nullptr;
        const uint32 indexCount = verticesData->getIndexCount();
        if (indexCount > 0)
        {
            indexBuffer = renderEngine->getBuffer();
            if ((indexBuffer == nullptr) || !indexBuffer->initGPU(sizeof(uint32) * vertexCount, verticesData->getIndices()))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create index buffer"));
                renderEngine->returnBuffer(indexBuffer);
                renderEngine->returnBuffer(vertexBuffer);
                return false;
            }
        }

        m_VertexBuffer = vertexBuffer;
        m_IndexBuffer = indexBuffer;
        m_CachedVertexSize = vertexDescription->size;
        m_RenderElementsCount = m_IndexBuffer != nullptr ? indexCount : vertexCount;
        return true;
    }

    void VertexBuffer_DirectX12::clearDirectX()
    {
        if (m_VertexBuffer != nullptr)
        {
            RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

            renderEngine->returnBuffer(m_VertexBuffer);
            renderEngine->returnBuffer(m_IndexBuffer);
            m_VertexBuffer = nullptr;
            m_IndexBuffer = nullptr;
        }
    }

    void VertexBuffer_DirectX12::render(const RenderOptions* renderOptions, Material* material)
    {
        Material_DirectX12* materialDirectX = dynamic_cast<Material_DirectX12*>(material);
        if (!materialDirectX->bindMaterial(renderOptions, this))
        {
            return;
        }

        const RenderOptions_DirectX12* renderOptionsDirectX = reinterpret_cast<const RenderOptions_DirectX12*>(renderOptions);
        ID3D12GraphicsCommandList2* commandList = renderOptionsDirectX->renderCommandList->get();

        commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
        vertexBufferView.BufferLocation = m_VertexBuffer->get()->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = m_VertexBuffer->getSize();
        vertexBufferView.StrideInBytes = m_CachedVertexSize;
        commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
        if (m_IndexBuffer != nullptr)
        {
            D3D12_INDEX_BUFFER_VIEW indexBufferView{};
            indexBufferView.BufferLocation = m_IndexBuffer->get()->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = m_IndexBuffer->getSize();
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;
            commandList->IASetIndexBuffer(&indexBufferView);

            commandList->DrawIndexedInstanced(m_RenderElementsCount, 1, 0, 0, 0);
        }
        else
        {
            commandList->DrawInstanced(m_RenderElementsCount, 1, 0, 0);
        }

        materialDirectX->unbindMaterial(renderOptions, this);
    }
}

#endif
