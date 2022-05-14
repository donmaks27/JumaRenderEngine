// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include "renderEngine/RenderEngine.h"

namespace JumaRenderEngine
{
    class RenderEngine_OpenGL : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_OpenGL() = default;
        virtual ~RenderEngine_OpenGL() override = default;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::OpenGL; }

        uint32 getTextureSamplerIndex(TextureSamplerType sampler);

    protected:

        virtual WindowController* createWindowController() override;
        virtual VertexBuffer* createVertexBufferInternal() override;
        virtual Texture* createTextureInternal() override;
        virtual Shader* createShaderInternal() override;
        virtual Material* createMaterialInternal() override;
        virtual RenderTarget* createRenderTargetInternal() override;

    private:

        jmap<TextureSamplerType, uint32> m_SamplerObjectIndices;
    };
}

#endif
