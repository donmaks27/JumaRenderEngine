// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderEngine.h"

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

        ID3D12Device2* getDevice() const { return m_Device; }
        DirectX12CommandQueue* getCommandQueue(const D3D12_COMMAND_LIST_TYPE queueType) const { return m_CommandQueues.find(queueType); }

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
        mutable jmap<D3D12_COMMAND_LIST_TYPE, DirectX12CommandQueue> m_CommandQueues;


        bool createDirectXDevice();
        bool createCommandQueues();

        void clearDirectX();
    };
}

#endif
