﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "TextureBase.h"

#include "jutils/math/vector2.h"
#include "texture/TextureFormat.h"
#include "texture/TextureSamples.h"
#include "window/window_id.h"

namespace JumaRenderEngine
{
    class WindowController;
    struct WindowData;
    struct RenderOptions;

    class RenderTarget : public TextureBase
    {
        friend RenderEngine;

    public:
        RenderTarget() = default;
        virtual ~RenderTarget() override;

        bool isWindowRenderTarget() const { return m_WindowID != window_id_INVALID; }
        window_id getWindowID() const { return m_WindowID; }
        TextureSamples getSampleCount() const { return m_TextureSamples; }

        math::uvector2 getSize() const { return m_Size; }
        TextureFormat getFormat() const { return m_Format; }

        virtual bool onStartRender(RenderOptions* renderOptions);
        virtual void onFinishRender(RenderOptions* renderOptions);

        void invalidate() { m_Invalid = true; }
        bool update();

    protected:

        virtual bool initInternal() { return true; }

        virtual bool recreateRenderTarget() { return false; }

    private:

        window_id m_WindowID = window_id_INVALID;
        TextureSamples m_TextureSamples = TextureSamples::X1;
        math::uvector2 m_Size = { 0, 0 };
        TextureFormat m_Format = TextureFormat::RGBA8;

        bool m_Invalid = true;


        bool init(window_id windowID, TextureSamples samples);
        bool init(TextureFormat format, const math::uvector2& size, TextureSamples samples);

        void clearData();

        void onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData);
    };
}
