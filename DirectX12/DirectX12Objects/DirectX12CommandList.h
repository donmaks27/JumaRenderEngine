// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <d3d12.h>

namespace JumaRenderEngine
{
    class DirectX12CommandQueue;

    class DirectX12CommandList
    {
        friend DirectX12CommandQueue;

    public:
        DirectX12CommandList() = default;
        ~DirectX12CommandList();

        ID3D12GraphicsCommandList2* get() const { return m_CommandList; }

        void execute();
        void waitForFinish() const;
        void markUnused();

    private:

        DirectX12CommandQueue* m_ParentCommandQueue = nullptr;

        ID3D12CommandAllocator* m_CommandAllocator = nullptr;
        ID3D12GraphicsCommandList2* m_CommandList = nullptr;
        uint64 m_FenceValue = 0;


        bool init(DirectX12CommandQueue* commandQueue);

        void clearDirectX();
    };
}

#endif
