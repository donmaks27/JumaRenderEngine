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
        if (!Super::initInternal())
        {
            return false;
        }
        createFramebuffers();
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->OnWindowPropertiesChanged.bind(this, &RenderTarget_OpenGL::onWindowPropertiesChanged);
        }
        return true;
    }
    void RenderTarget_OpenGL::createFramebuffers()
    {
        const TextureSamples sampleCount = getSampleCount();

        const bool renderToWindow = isWindowRenderTarget();
        const bool shouldResolveMultisampling = sampleCount != TextureSamples::X1;
        if (!shouldResolveMultisampling && renderToWindow)
        {
            return;
        }

        const GLenum colorFormat = GetOpenGLFormatByTextureFormat(getFormat());
        const GLenum depthFormat = GetOpenGLFormatByTextureFormat(TextureFormat::DEPTH24_STENCIL8);
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
                samplesNumber, colorFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)
            );
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorAttachment);
        }
        else
        {
            glGenTextures(1, &colorAttachment);
            glBindTexture(GL_TEXTURE_2D, colorAttachment);
            glTexImage2D(GL_TEXTURE_2D, 
                0, colorFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, 
                GL_RGB, GL_UNSIGNED_BYTE, nullptr
            );
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0);
        }
        if (depthEnabled)
        {
            glGenRenderbuffers(1, &depthAttachment);
            glBindRenderbuffer(GL_RENDERBUFFER, depthAttachment);
            if (shouldResolveMultisampling)
            {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER, 
                    samplesNumber, depthFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y)
                );
            }
            else
            {
                glRenderbufferStorage(GL_RENDERBUFFER, depthFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
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
                0, colorFormat, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr
            );
            glBindTexture(GL_TEXTURE_2D, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveAttachment, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_Framebuffer = framebufferIndices[0];
        m_ColorAttachment = colorAttachment;
        m_DepthAttachment = depthAttachment;
        m_ResolveFramebuffer = framebufferIndices[1];
        m_ResolveColorAttachment = resolveAttachment;
    }
    void RenderTarget_OpenGL::clearFramebuffers()
    {
        if (m_Framebuffer != 0)
        {
            getRenderEngine()->getWindowController<WindowController_OpenGL>()->setActiveWindowID(getWindowID());

            const GLuint framebuffers[] = { m_Framebuffer, m_ResolveFramebuffer };
            glDeleteFramebuffers(2, framebuffers);
            m_Framebuffer = 0;
            m_ResolveFramebuffer = 0;

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

    void RenderTarget_OpenGL::clearOpenGL()
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->OnWindowPropertiesChanged.unbind(this, &RenderTarget_OpenGL::onWindowPropertiesChanged);
        }
        clearFramebuffers();
    }

    void RenderTarget_OpenGL::onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData)
    {
        if ((windowData != nullptr) && (windowData->windowID == getWindowID()))
        {
            changeProperties(windowData->properties.size, windowData->properties.samples);
        }
    }
    void RenderTarget_OpenGL::onPropertiesChanged(const math::uvector2& prevSize, const TextureSamples prevSamples)
    {
        Super::onPropertiesChanged(prevSize, prevSamples);

        clearFramebuffers();
        createFramebuffers();
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
        glCullFace(GL_FRONT);

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

    bool RenderTarget_OpenGL::bindToShader(const uint32 bindIndex) const
    {
        const uint32 resultTextureIndex = getResultTextureIndex();
        return resultTextureIndex != 0 ? Texture_OpenGL::bindToShader(this, resultTextureIndex, bindIndex, getSamplerType()) : false;
    }
}

#endif
