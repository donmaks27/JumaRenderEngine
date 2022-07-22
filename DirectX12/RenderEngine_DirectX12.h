// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderEngine.h"

#include "D3D12MemAlloc.h"

#include "DirectX12Objects/DirectX12Buffer.h"
#include "DirectX12Objects/DirectX12CommandQueue.h"

namespace JumaRenderEngine
{
    class RenderEngine_DirectX12 final : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_DirectX12() = default;
        virtual ~RenderEngine_DirectX12() override;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::DirectX12; }

        virtual math::vector2 getScreenCoordinateModifier() const override { return { 1.0f, -1.0f }; }

        ID3D12Device2* getDevice() const { return m_Device; }
        D3D12MA::Allocator* getResourceAllocator() const { return m_ResourceAllocator; }
        DirectX12CommandQueue* getCommandQueue(const D3D12_COMMAND_LIST_TYPE queueType) const { return m_CommandQueues.find(queueType); }
        
        ID3D12DescriptorHeap* createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32 size, bool shaderVisible) const;
        template<D3D12_DESCRIPTOR_HEAP_TYPE Type>
        uint8 getDescriptorSize() const { return getCachedDescriptorSize<Type>(); }
        template<D3D12_DESCRIPTOR_HEAP_TYPE Type>
        D3D12_CPU_DESCRIPTOR_HANDLE getDescriptorCPU(ID3D12DescriptorHeap* descriptorHeap, const uint32 descriptorIndex) const
        {
            return { descriptorHeap != nullptr ? descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + descriptorIndex * getDescriptorSize<Type>() : 0 };
        }

        DirectX12Buffer* getBuffer();
        void returnBuffer(DirectX12Buffer* buffer);

    protected:

        virtual bool initInternal(const jmap<window_id, WindowProperties>& windows) override;
        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual VertexBuffer* createVertexBufferInternal() override;
        virtual Texture* createTextureInternal() override;
        virtual Shader* createShaderInternal() override;
        virtual Material* createMaterialInternal() override;
        virtual RenderTarget* createRenderTargetInternal() override;
        virtual RenderPipeline* createRenderPipelineInternal() override;

    private:

        ID3D12Device2* m_Device = nullptr;
        D3D12MA::Allocator* m_ResourceAllocator = nullptr;
        mutable jmap<D3D12_COMMAND_LIST_TYPE, DirectX12CommandQueue> m_CommandQueues;

        uint8 m_CachedDescriptorSize_RTV = 0;
        uint8 m_CachedDescriptorSize_DSV = 0;
        uint8 m_CachedDescriptorSize_SRV = 0;
        uint8 m_CachedDescriptorSize_Sampler = 0;

        jlist<DirectX12Buffer> m_Buffers;
        jarray<DirectX12Buffer*> m_UnusedBuffers;


        bool createDirectXDevice();
        bool createCommandQueues();

        void clearDirectX();

        template<D3D12_DESCRIPTOR_HEAP_TYPE Type>
        uint8 getCachedDescriptorSize() const { return 0; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>() const { return m_CachedDescriptorSize_RTV; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>() const { return m_CachedDescriptorSize_DSV; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>() const { return m_CachedDescriptorSize_SRV; }
        template<>
        uint8 getCachedDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>() const { return m_CachedDescriptorSize_Sampler; }
    };
}

#endif
