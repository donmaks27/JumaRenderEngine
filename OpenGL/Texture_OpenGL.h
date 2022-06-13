// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include "renderEngine/Texture.h"

#include <GL/glew.h>

namespace JumaRenderEngine
{
    constexpr GLenum GetOpenGLFormatByTextureFormat(const TextureFormat format)
    {
        switch (format)
        {
        case TextureFormat::RGBA_UINT8: return GL_RGBA;
        case TextureFormat::BGRA_UINT8: return GL_BGRA;
        case TextureFormat::DEPTH_FLOAT32: return GL_DEPTH_COMPONENT32F;
        case TextureFormat::DEPTH_UNORM24_STENCIL_UINT8: return GL_DEPTH24_STENCIL8;
        default: ;
        }
        return 0;
    }

    class Texture_OpenGL final : public Texture
    {
        using Super = Texture;

    public:
        Texture_OpenGL() = default;
        virtual ~Texture_OpenGL() override;

        bool bindTexture(const uint32 bindIndex) const { return bindTexture(this, m_TextureIndex, bindIndex, getSamplerType()); }
        static bool bindTexture(const RenderEngineContextObjectBase* contextObject, uint32 textureIndex, uint32 bindIndex, TextureSamplerType sampler);
        static void unbindTexture(uint32 bindIndex);

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) override;

    private:

        uint32 m_TextureIndex = 0;


        void clearOpenGL();
    };
}

#endif
