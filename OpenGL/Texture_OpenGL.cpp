// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Texture_OpenGL.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include "RenderEngine_OpenGL.h"

namespace JumaRenderEngine
{
    Texture_OpenGL::~Texture_OpenGL()
    {
        clearOpenGL();
    }

    bool Texture_OpenGL::initInternal(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        const GLenum formatOpenGL = GetOpenGLFormatByTextureFormat(format);
        if (formatOpenGL == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("Unsupported texture format"));
            return false;
        }

        glGenTextures(1, &m_TextureIndex);
        glBindTexture(GL_TEXTURE_2D, m_TextureIndex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, 
            static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, formatOpenGL, GL_UNSIGNED_BYTE, data
        );
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return true;
    }

    void Texture_OpenGL::clearOpenGL()
    {
        if (m_TextureIndex != 0)
        {
            glDeleteTextures(1, &m_TextureIndex);
            m_TextureIndex = 0;
        }
    }

    bool Texture_OpenGL::bindTexture(const RenderEngineContextObjectBase* contextObject, const uint32 textureIndex, const uint32 bindIndex, 
        const TextureSamplerType sampler)
    {
        if (textureIndex == 0)
        {
            return false;
        }

        const uint32 samplerIndex = contextObject->getRenderEngine<RenderEngine_OpenGL>()->getTextureSamplerIndex(sampler);

        glActiveTexture(GL_TEXTURE0 + bindIndex);
        glBindTexture(GL_TEXTURE_2D, textureIndex);
        glBindSampler(bindIndex, samplerIndex);
        return true;
    }
    void Texture_OpenGL::unbindTexture(const uint32 bindIndex)
    {
        glActiveTexture(GL_TEXTURE0 + bindIndex);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindSampler(bindIndex, 0);
    }
}

#endif
