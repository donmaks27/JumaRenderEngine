// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Texture_DirectX12.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "RenderEngine_DirectX12.h"
#include "TextureFormat_DirectX12.h"
#include "DirectX12Objects/DirectX12Texture.h"

namespace JumaRenderEngine
{
    Texture_DirectX12::~Texture_DirectX12()
    {
        clearDirectX();
    }

    bool Texture_DirectX12::initInternal(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

        DirectX12Texture* texture = renderEngine->createObject<DirectX12Texture>();
        texture->initColor(size, 1, GetDirectX12FormatByTextureFormat(format), 1, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAG_NONE);
        if (!texture->isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12Texture object"));
            delete texture;
            return false;
        }

        ID3D12Device2* device = renderEngine->getDevice();
        ID3D12Resource* textureResource = texture->getResource();
        const D3D12_RESOURCE_DESC resourceDescription = textureResource->GetDesc();
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
        device->GetCopyableFootprints(
            &resourceDescription, 0, 1, 0, 
            &footprint, nullptr, nullptr, nullptr
        );
        const uint32 bufferRowSize = footprint.Footprint.RowPitch;

        DirectX12Buffer* stagingBuffer = renderEngine->getBuffer();
        if ((stagingBuffer == nullptr) || !stagingBuffer->initStaging(bufferRowSize * size.y))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create staging buffer"));
            renderEngine->returnBuffer(stagingBuffer);
            delete texture;
            return false;
        }

        const int32 formatSize = GetTextureFormatSize(format);
        stagingBuffer->initMappedData();
        for (uint32 y = 0; y < size.y; y++)
        {
            std::memcpy(stagingBuffer->getMappedData(bufferRowSize * y), data + formatSize * size.x * y, formatSize * size.x);
        }
        stagingBuffer->flushMappedData(nullptr, true);

        DirectX12CommandQueue* commandQueue = renderEngine->getCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
        DirectX12CommandList* commandListObject = commandQueue->getCommandList();
        ID3D12GraphicsCommandList2* commandList = commandListObject->get();

        D3D12_TEXTURE_COPY_LOCATION srcCopyLocation{};
        srcCopyLocation.pResource = stagingBuffer->get();
        srcCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcCopyLocation.PlacedFootprint = footprint;
        D3D12_TEXTURE_COPY_LOCATION dstCopyLocation{};
        dstCopyLocation.pResource = textureResource;
        dstCopyLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstCopyLocation.SubresourceIndex = 0;
        commandList->CopyTextureRegion(&dstCopyLocation, 0, 0, 0, &srcCopyLocation, nullptr);
        /*D3D12_RESOURCE_BARRIER resourceBarrier{};
        resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        resourceBarrier.Transition.pResource = textureResource;
        resourceBarrier.Transition.Subresource = 0;
        resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        commandList->ResourceBarrier(1, &resourceBarrier);*/

        commandListObject->execute();

        ID3D12DescriptorHeap* srvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, false);
        if (srvDescriptorHeap != nullptr)
        {
            device->CreateShaderResourceView(textureResource, nullptr, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }

        commandListObject->waitForFinish();
        commandQueue->returnCommandList(commandListObject);
        renderEngine->returnBuffer(stagingBuffer);

        if (srvDescriptorHeap == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create SRV descriptor for texture"));
            delete texture;
            return false;
        }

        m_Texture = texture;
        m_DescriptorHeapSRV = srvDescriptorHeap;
        return true;
    }

    void Texture_DirectX12::clearDirectX()
    {
        if (m_DescriptorHeapSRV != nullptr)
        {
            m_DescriptorHeapSRV->Release();
            m_DescriptorHeapSRV = nullptr;
        }
        if (m_Texture != nullptr)
        {
            delete m_Texture;
            m_Texture = nullptr;
        }
    }
}

#endif
