// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderEngine_OpenGL.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include <GL/glew.h>

#include "Material_OpenGL.h"
#include "RenderTarget_OpenGL.h"
#include "Shader_OpenGL.h"
#include "Texture_OpenGL.h"
#include "VertexBuffer_OpenGL.h"
#include "renderEngine/window/OpenGL/WindowControllerInfo_OpenGL.h"

namespace JumaRenderEngine
{
    RenderEngine_OpenGL::~RenderEngine_OpenGL()
    {
        clearOpenGL();
    }

    void RenderEngine_OpenGL::clearInternal()
    {
        clearOpenGL();
        Super::clearInternal();
    }
    void RenderEngine_OpenGL::clearOpenGL()
    {
        clearRenderAssets();

        for (const auto& sampler : m_SamplerObjectIndices)
        {
            glDeleteSamplers(1, &sampler.value);
        }
        m_SamplerObjectIndices.clear();
    }

    WindowController* RenderEngine_OpenGL::createWindowController()
    {
        return WindowControllerInfo<RenderAPI::OpenGL>::create();
    }
    VertexBuffer* RenderEngine_OpenGL::createVertexBufferInternal()
    {
        return createObject<VertexBuffer_OpenGL>();
    }
    Texture* RenderEngine_OpenGL::createTextureInternal()
    {
        return createObject<Texture_OpenGL>();
    }
    Shader* RenderEngine_OpenGL::createShaderInternal()
    {
        return createObject<Shader_OpenGL>();
    }
    Material* RenderEngine_OpenGL::createMaterialInternal()
    {
        return createObject<Material_OpenGL>();
    }
    RenderTarget* RenderEngine_OpenGL::createRenderTargetInternal()
    {
        return createObject<RenderTarget_OpenGL>();
    }

    uint32 RenderEngine_OpenGL::getTextureSamplerIndex(const TextureSamplerType sampler)
    {
        const uint32* samplerIndexPtr = m_SamplerObjectIndices.find(sampler);
        if (samplerIndexPtr != nullptr)
        {
            return *samplerIndexPtr;
        }

        uint32 samplerIndex = 0;
        glGenSamplers(1, &samplerIndex);
        switch (sampler.filtering)
        {
        case TextureFiltering::Point:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            break;
        case TextureFiltering::Bilinear:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            break;
        case TextureFiltering::Trilinear:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
            break;
        case TextureFiltering::Anisotropic_2:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
            break;
        case TextureFiltering::Anisotropic_4:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
            break;
        case TextureFiltering::Anisotropic_8:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
            break;
        case TextureFiltering::Anisotropic_16:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glSamplerParameterf(samplerIndex, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
            break;
        default: ;
        }
        switch (sampler.wrapMode)
        {
        case TextureWrapMode::Repeat:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
        case TextureWrapMode::Mirror:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
            break;
        case TextureWrapMode::Clamp:
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glSamplerParameteri(samplerIndex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        default: ;
        }
        return m_SamplerObjectIndices[sampler] = samplerIndex;
    }
}

#endif
