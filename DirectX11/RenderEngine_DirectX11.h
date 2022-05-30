// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/RenderEngine.h"

namespace JumaRenderEngine
{
    class RenderEngine_DirectX11 final : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_DirectX11() = default;
        virtual ~RenderEngine_DirectX11() override;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::DirectX11; }

    protected:

        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual VertexBuffer* createVertexBufferInternal() override;
        virtual Texture* createTextureInternal() override;
        virtual Shader* createShaderInternal() override;
        virtual Material* createMaterialInternal() override;
        virtual RenderTarget* createRenderTargetInternal() override;

    private:

        void clearDirectX();
    };
}

#endif
