// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11.h>

#include "Material_DirectX11.h"
#include "RenderEngine_DirectX11.h"
#include "renderEngine/vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    VertexBuffer_DirectX11::~VertexBuffer_DirectX11()
    {
        clearDirectX();
    }

    bool VertexBuffer_DirectX11::initInternal(const VertexBufferData* verticesData)
    {
        const uint32 vertexCount = verticesData->getVertexCount();
        if (vertexCount == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("Empty vertex buffer data"));
            return false;
        }

        RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        const VertexDescription* vertexDescription = renderEngine->findVertexType(getVertexTypeName());
        ID3D11Device* device = renderEngine->getDevice();

        ID3D11Buffer* vertexBuffer = nullptr;
        D3D11_BUFFER_DESC vertexBufferDescription{};
        vertexBufferDescription.StructureByteStride = vertexDescription->size;
        vertexBufferDescription.ByteWidth = vertexDescription->size * vertexCount;
        vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDescription.CPUAccessFlags = 0;
        vertexBufferDescription.MiscFlags = 0;
        D3D11_SUBRESOURCE_DATA vertexBufferData{};
        vertexBufferData.pSysMem = verticesData->getVertices();
        HRESULT result = device->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &vertexBuffer);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 vertex buffer"));
            return false;
        }

        ID3D11Buffer* indexBuffer = nullptr;
        const uint32 indexCount = verticesData->getIndexCount();
        if (indexCount > 0)
        {
            D3D11_BUFFER_DESC indexBufferDescription{};
            indexBufferDescription.StructureByteStride = sizeof(uint32);
            indexBufferDescription.ByteWidth = sizeof(uint32) * indexCount;
            indexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
            indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
            indexBufferDescription.CPUAccessFlags = 0;
            indexBufferDescription.MiscFlags = 0;
            D3D11_SUBRESOURCE_DATA indexBufferData{};
            indexBufferData.pSysMem = verticesData->getIndices();
            result = device->CreateBuffer(&indexBufferDescription, &indexBufferData, &indexBuffer);
            if (result < 0)
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 index buffer"));
                vertexBuffer->Release();
                return false;
            }

            m_RenderElementsCount = indexCount;
        }
        else
        {
            m_RenderElementsCount = vertexCount;
        }

        m_VertexBuffer = vertexBuffer;
        m_IndexBuffer = indexBuffer;
        m_VertexSize = vertexDescription->size;
        return true;
    }

    void VertexBuffer_DirectX11::clearDirectX()
    {
        if (m_IndexBuffer != nullptr)
        {
            m_IndexBuffer->Release();
            m_IndexBuffer = nullptr;
        }
        if (m_VertexBuffer != nullptr)
        {
            m_VertexBuffer->Release();
            m_VertexBuffer = nullptr;
        }
        m_RenderElementsCount = 0;
        m_VertexSize = 0;
    }

    void VertexBuffer_DirectX11::render(const RenderOptions* renderOptions, Material* material)
    {
        Material_DirectX11* materialDirectX = dynamic_cast<Material_DirectX11*>(material);
        if (!materialDirectX->bindMaterial(renderOptions, this))
        {
            return;
        }

        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();

        static constexpr UINT vertexOffet = 0;
        deviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &m_VertexSize, &vertexOffet);
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        if (m_IndexBuffer != nullptr)
        {
            deviceContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
            deviceContext->DrawIndexed(m_RenderElementsCount, 0, 0);
        }
        else
        {
            deviceContext->Draw(m_RenderElementsCount, 0);
        }

        materialDirectX->unbindMaterial(renderOptions, this);
    }
}

#endif
