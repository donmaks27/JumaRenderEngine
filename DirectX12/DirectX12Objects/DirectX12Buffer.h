// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderEngineContextObject.h"

#include "renderEngine/DirectX12/D3D12MemAlloc.h"

namespace JumaRenderEngine
{
    class DirectX12CommandList;

    class DirectX12Buffer : public RenderEngineContextObject
    {
    public:
        DirectX12Buffer() = default;
        virtual ~DirectX12Buffer() override;

        // Temp buffer for passing data to GPU
        bool initStaging(uint32 size);
        // Only on GPU, not updated at all
        bool initGPU(uint32 size, const void* data, D3D12_RESOURCE_STATES bufferState);
        // GPU buffer, frequently writing from CPU. GPU with staging buffer
        bool initAccessedGPU(uint32 size, D3D12_RESOURCE_STATES bufferState);

        ID3D12Resource* get() const { return m_Buffer; }
        uint32 getSize() const { return m_BufferSize; }
        
        bool initMappedData();
        void* getMappedData(uint32 offset) const;
        bool setMappedData(const void* data, uint32 size, uint32 offset = 0) const;
        bool flushMappedData(DirectX12CommandList* commandList, bool waitForFinish);
        
        bool setData(const void* data, uint32 size, uint32 offset, bool waitForFinish);

    protected:

        virtual void clearInternal() override { clearDirectX(); }

    private:

        D3D12MA::Allocation* m_Allocation = nullptr;
        ID3D12Resource* m_Buffer = nullptr;

        uint32 m_BufferSize = 0;
        D3D12_RESOURCE_STATES m_BufferState = D3D12_RESOURCE_STATE_COMMON;

        DirectX12Buffer* m_StagingBuffer = nullptr;
        void* m_MappedData = nullptr;
        bool m_Mapable = false;


        void clearDirectX();

        bool setDataInternal(const void* data, uint32 size, uint32 offset);
        bool copyData(DirectX12CommandList* commandList, const DirectX12Buffer* destinationBuffer, bool shouldChangeState, bool waitForFinish);
    };
}

#endif
