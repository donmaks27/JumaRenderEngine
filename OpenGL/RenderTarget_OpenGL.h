// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include "renderEngine/RenderTarget.h"

namespace JumaRenderEngine
{
    class RenderTarget_OpenGL : public RenderTarget
    {
        using Super = RenderTarget;

    public:
        RenderTarget_OpenGL() = default;
        virtual ~RenderTarget_OpenGL() override;

        uint32 getResultTextureIndex() const { return !isWindowRenderTarget() ? (m_ResolveColorAttachment != 0 ? m_ResolveColorAttachment : m_ColorAttachment) : 0; }

        virtual void onStartRender() override;
        virtual void onFinishRender() override;

        bool bindResultTexture(uint32 bindIndex) const;

    protected:

        virtual bool initInternal() override;

    private:

        uint32 m_ColorAttachment = 0;
        uint32 m_DepthAttachment = 0;
        uint32 m_ResolveColorAttachment = 0;

        uint32 m_Framebuffer = 0;
        uint32 m_ResolveFramebuffer = 0;


        bool createFramebuffers();

        void clearOpenGL();
    };
}

#endif
