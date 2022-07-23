// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "DirectX12CommandQueue.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/DirectX12/RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    DirectX12CommandQueue::~DirectX12CommandQueue()
    {
        clearDirectX();
    }

    bool DirectX12CommandQueue::init(const D3D12_COMMAND_LIST_TYPE queueType)
    {
        ID3D12Device2* device = getRenderEngine<RenderEngine_DirectX12>()->getDevice();

        D3D12_COMMAND_QUEUE_DESC commandQueueDescription{};
        commandQueueDescription.Type = queueType;
        commandQueueDescription.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        commandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        commandQueueDescription.NodeMask = 0;
        ID3D12CommandQueue* commandQueue = nullptr;
        HRESULT result = device->CreateCommandQueue(&commandQueueDescription, IID_PPV_ARGS(&commandQueue));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 command queue"));
            return false;
        }

        ID3D12Fence* fence = nullptr;
        result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 fence"));
            commandQueue->Release();
            return false;
        }

        HANDLE fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (fenceEvent == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create fence event"));
            fence->Release();
            commandQueue->Release();
            return false;
        }

        m_Queue = commandQueue;
        m_QueueType = queueType;
        m_Fence = fence;
        m_FenceEvent = fenceEvent;
        m_LastFenceValue = 0;
        return true;
    }

    void DirectX12CommandQueue::clearDirectX()
    {
        m_LastFenceValue = 0;

        m_UnusedCommandLists.clear();
        m_CommandLists.clear();

        if (m_FenceEvent != nullptr)
        {
            ::CloseHandle(m_FenceEvent);
            m_FenceEvent = nullptr;
        }
        if (m_Fence != nullptr)
        {
            m_Fence->Release();
            m_Fence = nullptr;
        }
        if (m_Queue != nullptr)
        {
            m_Queue->Release();
            m_Queue = nullptr;
        }
    }

    uint64 DirectX12CommandQueue::signalFence()
    {
        m_Queue->Signal(m_Fence, ++m_LastFenceValue);
        return m_LastFenceValue;
    }
    void DirectX12CommandQueue::waitForFenceValue(const uint64 fenceValue) const
    {
        if (isFenceValueReached(fenceValue))
        {
            return;
        }

        m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent);
        ::WaitForSingleObject(m_FenceEvent, INFINITE);
    }

    DirectX12CommandList* DirectX12CommandQueue::getCommandList()
    {
        if (!m_UnusedCommandLists.isEmpty())
        {
            for (int32 index = 0; index < m_UnusedCommandLists.getSize(); index++)
            {
                DirectX12CommandList* commandList = m_UnusedCommandLists[index];
                if (commandList->isValidForReuse())
                {
                    m_UnusedCommandLists.removeAt(index);
                    commandList->reset();
                    return commandList;
                }
            }
        }

        DirectX12CommandList* commandList = &m_CommandLists.addDefault();
        if (!commandList->init(this))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create command list"));
            m_CommandLists.removeLast();
            return nullptr;
        }
        return commandList;
    }
    void DirectX12CommandQueue::returnCommandList(DirectX12CommandList* commandList)
    {
        if (commandList != nullptr)
        {
            m_UnusedCommandLists.add(commandList);
        }
    }
}

#endif
