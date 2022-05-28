// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderTarget_OpenGL.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include <GL/glew.h>

#include "Texture_OpenGL.h"
#include "renderEngine/RenderEngine.h"
#include "renderEngine/window/OpenGL/WindowController_OpenGL.h"

namespace JumaRenderEngine
{
    RenderTarget_OpenGL::~RenderTarget_OpenGL()
    {
        clearOpenGL();
    }

    bool RenderTarget_OpenGL::initInternal()
    {
        if (!createFramebuffers())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create OpenGL framebuffers"));
            return false;
        }
        return true;
    }
    bool RenderTarget_OpenGL::createFramebuffers()
    {
        const TextureSamples sampleCount = getSampleCount();

        const bool renderToWindow = isWindowRenderTarget();
        const bool shouldResolveMultisampling = sampleCount != TextureSamples::X1;
        if (!shouldResolveMultisampling && renderToWindow)
        {
            return true;
        }

        const uint32 textureFormat = GetOpenGLFormatByTextureFormat(getFormat());
        if (textureFormat == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("Unsupported render target format"));
            return false;
        }

        const uint8 samplesNumber = GetTextureSamplesNumber(sampleCount);
        const bool depthEnabled = true;
        const bool resolveFramebufferEnabled = !renderToWindow && shouldResolveMultisampling;
        const math::uvector2 size = getSize();

        GLuint colorAttachment = 0, depthAttachment = 0, resolveAttachment = 0;
        GLuint framebufferIndices[2] = { 0, 0 };

        getRenderEngine()->getWindowController<WindowController_OpenGL>()->setActiveWindowID(getWindowID());
        glGenFramebuffers(resolveFramebufferEnabled ? 2 : 1, framebufferIndices);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferIndices[0]);
        if (shouldResolveMultisampling)
        {
            glGenRenderbuffers(1, &colorAttachment);
            glBindRenderbuffer(GL_RENDERBUFFER, colorAttachment);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                static_cast<GLsizei>(samplesNumber), textureFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)
            );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorAttachment);
        }
        else
        {
            glGenTextures(1, &colorAttachment);
            glBindTexture(GL_TEXTURE_2D, colorAttachment);
            glTexImage2D(GL_TEXTURE_2D, 
                0, textureFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, 
                GL_RGB, GL_UNSIGNED_BYTE, nullptr
            );
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0);
        }
        if (depthEnabled)
        {
            glGenRenderbuffers(1, &depthAttachment);
            glBindRenderbuffer(GL_RENDERBUFFER, depthAttachment);
            if (samplesNumber == 1)
            {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
            }
            else
            {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
                    static_cast<GLsizei>(samplesNumber), GL_DEPTH24_STENCIL8, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)
                );
            }
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);
        }
        if (resolveFramebufferEnabled)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferIndices[1]);
            glGenTextures(1, &resolveAttachment);
            glBindTexture(GL_TEXTURE_2D, resolveAttachment);
            glTexImage2D(GL_TEXTURE_2D, 
                0, textureFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr
            );
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveAttachment, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_Framebuffer = framebufferIndices[0];
        m_ColorAttachment = colorAttachment;
        m_DepthAttachment = depthAttachment;
        m_ResolveFramebuffer = framebufferIndices[1];
        m_ResolveColorAttachment = resolveAttachment;
        return true;
    }

    void RenderTarget_OpenGL::clearOpenGL()
    {
        if (m_Framebuffer != 0)
        {
            getRenderEngine()->getWindowController<WindowController_OpenGL>()->setActiveWindowID(getWindowID());

            glDeleteFramebuffers(1, &m_Framebuffer);
            m_Framebuffer = 0;
            if (m_ResolveFramebuffer != 0)
            {
                glDeleteFramebuffers(1, &m_ResolveFramebuffer);
                m_ResolveFramebuffer = 0;
            }

            const bool shouldResolveMultisampling = getSampleCount() != TextureSamples::X1;
            if (shouldResolveMultisampling)
            {
                glDeleteRenderbuffers(1, &m_ColorAttachment);
            }
            else
            {
                glDeleteTextures(1, &m_ColorAttachment);
            }
            m_ColorAttachment = 0;

            if (m_DepthAttachment != 0)
            {
                glDeleteRenderbuffers(1, &m_DepthAttachment);
                m_DepthAttachment = 0;
            }
            if (m_ResolveColorAttachment != 0)
            {
                glDeleteTextures(1, &m_ResolveColorAttachment);
                m_ResolveColorAttachment = 0;
            }
        }
    }

    bool RenderTarget_OpenGL::onStartRender(RenderOptions* renderOptions)
    {
        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }

        getRenderEngine()->getWindowController<WindowController_OpenGL>()->setActiveWindowID(getWindowID());
        glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        const math::uvector2 size = getSize();
        glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
        return true;
    }
    void RenderTarget_OpenGL::onFinishRender(RenderOptions* renderOptions)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (getSampleCount() != TextureSamples::X1)
        {
            const math::uvector2 size = getSize();

            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_Framebuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ResolveFramebuffer);
            glBlitFramebuffer(
                0, 0, static_cast<GLint>(size.x), static_cast<GLint>(size.y), 
                0, 0, static_cast<GLint>(size.x), static_cast<GLint>(size.y), 
                GL_COLOR_BUFFER_BIT, GL_NEAREST
            );
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        const uint32 resultTextureIndex = getResultTextureIndex();
        if (resultTextureIndex != 0)
        {
            glBindTexture(GL_TEXTURE_2D, resultTextureIndex);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        Super::onFinishRender(renderOptions);
    }

    bool RenderTarget_OpenGL::bindResultTexture(const uint32 bindIndex) const
    {
        const uint32 resultTextureIndex = getResultTextureIndex();
        return resultTextureIndex != 0 ? Texture_OpenGL::bindTexture(this, resultTextureIndex, bindIndex, getSamplerType()) : false;
    }
}

#endif
