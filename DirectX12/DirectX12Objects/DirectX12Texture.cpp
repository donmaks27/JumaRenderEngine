// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "DirectX12Texture.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/DirectX12/RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    DirectX12Texture::~DirectX12Texture()
    {
        clearDirectX();
    }

    bool DirectX12Texture::initColor(const math::uvector2& size, const uint32 sampleCount, const DXGI_FORMAT format, const uint32 mipLevels, 
        const D3D12_RESOURCE_STATES initialState, const D3D12_RESOURCE_FLAGS flags)
    {
        if (isValid())
        {
            JUMA_RENDER_LOG(warning, JSTR("DirectX12 texture already initialized"));
            return false;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        D3D12MA::Allocator* allocator = renderEngine->getResourceAllocator();

        D3D12MA::ALLOCATION_DESC allocationDescription{};
        allocationDescription.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_DESC resourceDescription{};
        resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDescription.Alignment = 0;
        resourceDescription.Width = size.x;
        resourceDescription.Height = size.y;
        resourceDescription.DepthOrArraySize = 1;
        resourceDescription.MipLevels = static_cast<UINT16>(mipLevels);
        resourceDescription.Format = format;
        resourceDescription.SampleDesc.Count = sampleCount;
        resourceDescription.SampleDesc.Quality = 0;
        resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDescription.Flags = flags;
        D3D12MA::Allocation* allocation = nullptr;
        ID3D12Resource* resource = nullptr;
        const HRESULT result = allocator->CreateResource(&allocationDescription, &resourceDescription, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 texture resource"));
            return false;
        }

        m_Allocation = allocation;
        m_TextureResource = resource;
        m_Size = size;
        m_Format = format;
        m_MipLevels = static_cast<uint8>(m_TextureResource->GetDesc().MipLevels);
        m_CurrentSubresourcesState = jarray(m_MipLevels, initialState);
        markAsInitialized();
        return true;
    }
    bool DirectX12Texture::initDepth(const math::uvector2& size, const uint32 sampleCount, const DXGI_FORMAT format,
        const D3D12_RESOURCE_STATES initialState, const D3D12_RESOURCE_FLAGS flags)
    {
        if (isValid())
        {
            JUMA_RENDER_LOG(warning, JSTR("DirectX12 texture already initialized"));
            return false;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        D3D12MA::Allocator* allocator = renderEngine->getResourceAllocator();

        D3D12MA::ALLOCATION_DESC allocationDescription{};
        allocationDescription.HeapType = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_DESC resourceDescription{};
        resourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resourceDescription.Alignment = 0;
        resourceDescription.Width = size.x;
        resourceDescription.Height = size.y;
        resourceDescription.DepthOrArraySize = 1;
        resourceDescription.MipLevels = 1;
        resourceDescription.Format = format;
        resourceDescription.SampleDesc.Count = sampleCount;
        resourceDescription.SampleDesc.Quality = 0;
        resourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDescription.Flags = flags | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format = resourceDescription.Format;
        clearValue.DepthStencil.Depth = 1.0f;
        clearValue.DepthStencil.Stencil = 0;
        D3D12MA::Allocation* allocation = nullptr;
        ID3D12Resource* resource = nullptr;
        const HRESULT result = allocator->CreateResource(&allocationDescription, &resourceDescription, initialState, &clearValue, &allocation, IID_PPV_ARGS(&resource));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 texture resource"));
            return false;
        }

        m_Allocation = allocation;
        m_TextureResource = resource;
        m_Size = size;
        m_Format = format;
        m_MipLevels = 1;
        m_CurrentSubresourcesState = jarray(m_MipLevels, initialState);
        markAsInitialized();
        return true;
    }

    bool DirectX12Texture::init(ID3D12Resource* existingResource, const D3D12_RESOURCE_STATES currentState)
    {
        if (isValid())
        {
            JUMA_RENDER_LOG(warning, JSTR("DirectX12 texture already initialized"));
            return false;
        }
        if (existingResource == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid DirectX12 resource object"));
            return false;
        }

        const D3D12_RESOURCE_DESC resourceDescription = existingResource->GetDesc();
        m_TextureResource = existingResource;
        m_Size = { static_cast<uint32>(resourceDescription.Width), static_cast<uint32>(resourceDescription.Height) };
        m_Format = resourceDescription.Format;
        m_MipLevels = static_cast<uint8>(resourceDescription.MipLevels);
        m_CurrentSubresourcesState = jarray(m_MipLevels, currentState);
        markAsInitialized();
        return true;
    }

    void DirectX12Texture::clearDirectX()
    {
        if (m_Allocation != nullptr)
        {
            if (m_TextureResource != nullptr)
            {
                m_TextureResource->Release();
                m_TextureResource = nullptr;
            }
            m_Allocation->Release();
            m_Allocation = nullptr;
        }
        else
        {
            m_TextureResource = nullptr;
        }
    }

    void DirectX12Texture::setMipLevelState(const uint8 mipLevelIndex, const D3D12_RESOURCE_STATES state)
    {
        if (m_CurrentSubresourcesState.isValidIndex(mipLevelIndex))
        {
            m_CurrentSubresourcesState[mipLevelIndex] = state;
        }
    }
    void DirectX12Texture::setMipLevelsState(jarray<D3D12_RESOURCE_STATES>&& states)
    {
        if (states.getSize() == m_MipLevels)
        {
            m_CurrentSubresourcesState = std::move(states);
        }
        else
        {
            JUMA_RENDER_LOG(warning, JSTR("Invalid input data"));
        }
    }
    void DirectX12Texture::setMipLevelsState(const D3D12_RESOURCE_STATES state)
    {
        for (uint8 index = 0; index < m_MipLevels; index++)
        {
            m_CurrentSubresourcesState[index] = state;
        }
    }
}

#endif
