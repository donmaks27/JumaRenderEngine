// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "DirectX12MipGeneratorTarget.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <d3d12.h>

#include "DirectX12MipGenerator.h"
#include "DirectX12Texture.h"
#include "renderEngine/DirectX12/RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    bool DirectX12MipGeneratorTarget::initMipGeneratorTarget()
    {
        const DirectX12Texture* targetTexture = getMipGeneratorTargetTexture();
        const uint8 mipLevelsCount = targetTexture != nullptr ? targetTexture->getMipLevelsCount() : 0;
        if (mipLevelsCount <= 1)
        {
            return true;
        }
        const DXGI_FORMAT targetTextureFormat = targetTexture->getFormat();

        RenderEngine_DirectX12* renderEngine = targetTexture->getRenderEngine<RenderEngine_DirectX12>();
        DirectX12Texture* stagingTexture = renderEngine->createObject<DirectX12Texture>();
        if (!stagingTexture->initColor(targetTexture->getSize(), 1, targetTextureFormat, mipLevelsCount, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create staging texture"));
            delete stagingTexture;
            return false;
        }

        const uint8 stagesCount = static_cast<uint8>(((mipLevelsCount - 1) / 4) + (((mipLevelsCount - 1) % 4) != 0 ? 1 : 0));
        const uint32 descriptorHeapSize = stagesCount * 4 + 1;
        ID3D12DescriptorHeap* descriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorHeapSize, true);
        if (descriptorHeap == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create descriptor heap for mip generator target"));
            delete stagingTexture;
            return false;
        }
        const uint8 descriptorSize = renderEngine->getDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>();
        D3D12_CPU_DESCRIPTOR_HANDLE descriptor = descriptorHeap->GetCPUDescriptorHandleForHeapStart();

        ID3D12Device2* device = renderEngine->getDevice();
        ID3D12Resource* stagingTextureResource = stagingTexture->getResource();
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription{};
        srvDescription.Format = targetTextureFormat;
        srvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDescription.Texture2D.MostDetailedMip = 0;
        srvDescription.Texture2D.MipLevels = mipLevelsCount;
        srvDescription.Texture2D.PlaneSlice = 0;
        srvDescription.Texture2D.ResourceMinLODClamp = 0;
        device->CreateShaderResourceView(stagingTextureResource, &srvDescription, descriptor);

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDescription{};
        uavDescription.Format = targetTextureFormat;
        uavDescription.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavDescription.Texture2D.PlaneSlice = 0;
        for (uint32 mipIndex = 1; mipIndex < mipLevelsCount; mipIndex++)
        {
            uavDescription.Texture2D.MipSlice = mipIndex;
            descriptor.ptr += descriptorSize;
            device->CreateUnorderedAccessView(stagingTextureResource, nullptr, &uavDescription, descriptor);
        }

        m_StagingTexture = stagingTexture;
        m_DescriptorHeap_SRV_UAV = descriptorHeap;
        m_GeneratingStagesCount = stagesCount;
        return true;
    }

    void DirectX12MipGeneratorTarget::clearMipGeneratorTarget()
    {
        if (m_DescriptorHeap_SRV_UAV != nullptr)
        {
            m_DescriptorHeap_SRV_UAV->Release();
            m_DescriptorHeap_SRV_UAV = nullptr;
        }
        if (m_StagingTexture != nullptr)
        {
            delete m_StagingTexture;
            m_StagingTexture = nullptr;
        }
    }
}

#endif
