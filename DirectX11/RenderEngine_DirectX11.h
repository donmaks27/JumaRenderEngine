// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/RenderEngine.h"

#include "renderEngine/texture/TextureSamplerType.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11SamplerState;

namespace JumaRenderEngine
{
    class RenderEngine_DirectX11 final : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_DirectX11() = default;
        virtual ~RenderEngine_DirectX11() override;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::DirectX11; }

        ID3D11Device* getDevice() const { return m_Device; }
        ID3D11DeviceContext* getDeviceContext() const { return m_DeviceContext; }

        ID3D11SamplerState* getTextureSampler(TextureSamplerType samplerType);

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

        ID3D11Device* m_Device = nullptr;
        ID3D11DeviceContext* m_DeviceContext = nullptr;

        jmap<TextureSamplerType, ID3D11SamplerState*> m_TextureSamplers;


        bool createDirectXDevice();

        void clearDirectX();
    };
}

#endif
