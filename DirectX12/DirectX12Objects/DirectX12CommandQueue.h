// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderEngineContextObject.h"

#include <d3d12.h>

#include "DirectX12CommandList.h"
#include "jutils/jarray.h"
#include "jutils/jlist.h"

namespace JumaRenderEngine
{
    class RenderEngine_DirectX12;

    class DirectX12CommandQueue : public RenderEngineContextObjectBase
    {
        friend RenderEngine_DirectX12;

    public:
        DirectX12CommandQueue() = default;
        virtual ~DirectX12CommandQueue() override;

        ID3D12CommandQueue* get() const { return m_Queue; }
        D3D12_COMMAND_LIST_TYPE getType() const { return m_QueueType; }

        uint64 signalFence();
        bool isFenceValueReached(const uint64 fenceValue) const { return m_Fence->GetCompletedValue() >= fenceValue; }
        void waitForFenceValue(uint64 fenceValue) const;
        void waitForFinish() { waitForFenceValue(signalFence()); }

        DirectX12CommandList* getCommandList();
        void returnCommandList(DirectX12CommandList* commandList);

    private:

        ID3D12CommandQueue* m_Queue = nullptr;
        D3D12_COMMAND_LIST_TYPE m_QueueType = D3D12_COMMAND_LIST_TYPE_DIRECT;

        ID3D12Fence* m_Fence = nullptr;
        HANDLE m_FenceEvent = nullptr;
        uint64 m_LastFenceValue = 0;

        jlist<DirectX12CommandList> m_CommandLists;
        jarray<DirectX12CommandList*> m_UnusedCommandLists;


        bool init(D3D12_COMMAND_LIST_TYPE queueType);

        void clearDirectX();
    };
}

#endif
