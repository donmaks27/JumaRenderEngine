// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "TextureBase.h"

#include "jutils/math/vector2.h"
#include "texture/TextureFormat.h"
#include "texture/TextureSamples.h"
#include "window/window_id.h"

namespace JumaRenderEngine
{
    class RenderTarget : public TextureBase
    {
        friend RenderEngine;

    public:
        RenderTarget() = default;
        virtual ~RenderTarget() override = default;

        bool isWindowRenderTarget() const { return m_WindowID != window_id_INVALID; }
        window_id getWindowID() const { return m_WindowID; }
        TextureSamples getSampleCount() const { return m_TextureSamples; }

        math::uvector2 getSize() const;
        TextureFormat getFormat() const { return m_Format; }

        virtual void onStartRender();
        virtual void onFinishRender();

    protected:

        virtual bool initInternal() { return true; }

    private:

        window_id m_WindowID = window_id_INVALID;
        TextureSamples m_TextureSamples = TextureSamples::X1;

        // Ignore it if window ID is valid
        math::uvector2 m_Size = { 0, 0 };
        TextureFormat m_Format = TextureFormat::RGBA_UINT8;


        bool init(window_id windowID, TextureSamples samples);
        bool init(TextureFormat format, const math::uvector2& size, TextureSamples samples);
    };
}
