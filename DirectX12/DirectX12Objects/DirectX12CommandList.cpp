// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "DirectX12CommandList.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "DirectX12CommandQueue.h"
#include "DirectX12Texture.h"
#include "renderEngine/DirectX12/RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    DirectX12CommandList::~DirectX12CommandList()
    {
        clearDirectX();
    }

    bool DirectX12CommandList::init(DirectX12CommandQueue* commandQueue)
    {
        ID3D12Device2* device = commandQueue->getRenderEngine<RenderEngine_DirectX12>()->getDevice();
        const D3D12_COMMAND_LIST_TYPE type = commandQueue->getType();

        ID3D12CommandAllocator* allocator = nullptr;
        HRESULT result = device->CreateCommandAllocator(type, IID_PPV_ARGS(&allocator));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 command allocator"));
            return false;
        }

        ID3D12GraphicsCommandList2* commandList = nullptr;
        result = device->CreateCommandList(0, type, allocator, nullptr, IID_PPV_ARGS(&commandList));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 command list"));
            allocator->Release();
            return false;
        }

        m_ParentCommandQueue = commandQueue;
        m_CommandAllocator = allocator;
        m_CommandList = commandList;
        return true;
    }

    void DirectX12CommandList::clearDirectX()
    {
        m_TextureStates.clear();
        m_ResourceBarriers.clear();

        if (m_CommandList != nullptr)
        {
            m_CommandList->Release();
            m_CommandList = nullptr;
        }
        if (m_CommandAllocator != nullptr)
        {
            m_CommandAllocator->Release();
            m_CommandAllocator = nullptr;
        }

        m_ParentCommandQueue = nullptr;
        m_FenceValue = 0;
    }

    void DirectX12CommandList::execute()
    {
        m_CommandList->Close();

        ID3D12CommandList* const commandLists[] = { m_CommandList };
        m_ParentCommandQueue->get()->ExecuteCommandLists(1, commandLists);
        m_FenceValue = m_ParentCommandQueue->signalFence();

        for (const auto& textureState : m_TextureStates)
        {
            textureState.key->setState(textureState.value);
        }
        m_TextureStates.clear();
    }

    void DirectX12CommandList::waitForFinish() const
    {
        m_ParentCommandQueue->waitForFenceValue(m_FenceValue);
    }

    void DirectX12CommandList::markUnused()
    {
        m_ParentCommandQueue->returnCommandList(this);
    }

    void DirectX12CommandList::reset()
    {
        m_ResourceBarriers.clear();
        m_TextureStates.clear();

        m_CommandAllocator->Reset();
        m_CommandList->Reset(m_CommandAllocator, nullptr);
    }

    void DirectX12CommandList::changeTextureState(DirectX12Texture* texture, const D3D12_RESOURCE_STATES state)
    {
        if ((texture == nullptr) || !texture->isValid())
        {
            return;
        }

        const D3D12_RESOURCE_STATES* prevStatePtr = m_TextureStates.find(texture);
        const D3D12_RESOURCE_STATES prevState = prevStatePtr != nullptr ? *prevStatePtr : texture->getState();
        if (prevState == state)
        {
            return;
        }

        D3D12_RESOURCE_BARRIER& resourceBarrier = m_ResourceBarriers.addDefault();
        resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        resourceBarrier.Transition.pResource = texture->getResource();
        resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        resourceBarrier.Transition.StateBefore = prevState;
        resourceBarrier.Transition.StateAfter = state;
        m_TextureStates[texture] = state;
    }
    void DirectX12CommandList::applyTextureStateChanges()
    {
        if (!m_ResourceBarriers.isEmpty())
        {
            m_CommandList->ResourceBarrier(m_ResourceBarriers.getSize(), m_ResourceBarriers.getData());
            m_ResourceBarriers.clear();
        }
    }
}

#endif
