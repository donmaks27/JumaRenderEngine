// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderEngineContextObject.h"

#include "renderEngine/DirectX12/D3D12MemAlloc.h"

#include "jutils/jarray.h"
#include "jutils/math/vector2.h"

namespace JumaRenderEngine
{
    class DirectX12Texture : public RenderEngineContextObject
    {
    public:
        DirectX12Texture() = default;
        virtual ~DirectX12Texture() override;

        bool initColor(const math::uvector2& size, uint32 sampleCount, DXGI_FORMAT format, uint32 mipLevels = 0, 
            D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
        bool initDepth(const math::uvector2& size, uint32 sampleCount, DXGI_FORMAT format, 
            D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
        bool init(ID3D12Resource* existingResource, D3D12_RESOURCE_STATES currentState);

        ID3D12Resource* getResource() const { return m_TextureResource; }
        math::uvector2 getSize() const { return m_Size; }
        DXGI_FORMAT getFormat() const { return m_Format; }
        uint8 getMipLevelsCount() const { return m_MipLevels; }

        D3D12_RESOURCE_STATES getMipLevelState(const uint8 mipLevelIndex) const { return m_CurrentSubresourcesState.isValidIndex(mipLevelIndex) ? m_CurrentSubresourcesState[mipLevelIndex] : D3D12_RESOURCE_STATE_COMMON; }
        const jarray<D3D12_RESOURCE_STATES>& getMipLevelsState() const { return m_CurrentSubresourcesState; }
        void setMipLevelState(uint8 mipLevelIndex, D3D12_RESOURCE_STATES state);
        void setMipLevelsState(jarray<D3D12_RESOURCE_STATES>&& states);
        void setMipLevelsState(D3D12_RESOURCE_STATES state);

    protected:

        virtual void clearInternal() override { clearDirectX(); }

    private:

        D3D12MA::Allocation* m_Allocation = nullptr;
        ID3D12Resource* m_TextureResource = nullptr;

        math::uvector2 m_Size = { 0, 0 };
        DXGI_FORMAT m_Format = DXGI_FORMAT_UNKNOWN;
        uint8 m_MipLevels = 0;

        jarray<D3D12_RESOURCE_STATES> m_CurrentSubresourcesState;


        void clearDirectX();
    };
}

#endif
