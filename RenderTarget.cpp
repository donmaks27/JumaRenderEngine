// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderTarget.h"

#include "RenderEngine.h"

namespace JumaRenderEngine
{
    RenderTarget::~RenderTarget()
    {
        clearData();
    }

    bool RenderTarget::init(const window_id windowID, const TextureSamples samples)
    {
        if (getRenderEngine()->getWindowController()->findWindowData(windowID) == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("There is not window ") + TO_JSTR(windowID));
            return false;
        }

        m_WindowID = windowID;
        m_TextureSamples = samples;
        if (!initInternal())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize window render target"));
            clearData();
            return false;
        }
        return true;
    }
    bool RenderTarget::init(const TextureFormat format, const math::uvector2& size, const TextureSamples samples)
    {
        if ((size.x == 0) || (size.y == 0))
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid input params"));
            return false;
        }

        m_Format = format;
        m_Size = size;
        m_TextureSamples = samples;
        if (!initInternal())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize window render target"));
            clearData();
            return false;
        }
        return true;
    }

    void RenderTarget::clearData()
    {
        m_WindowID = window_id_INVALID;
        m_TextureSamples = TextureSamples::X1;
        m_Size = { 0, 0 };
        m_Format = TextureFormat::RGBA_UINT8;
    }

    math::uvector2 RenderTarget::getSize() const
    {
        if (!isWindowRenderTarget())
        {
            return m_Size;
        }
        const WindowData* window = getRenderEngine()->getWindowController()->findWindowData(getWindowID());
        return window != nullptr ? window->size : math::uvector2(0);
    }

    bool RenderTarget::onStartRender(RenderOptions* renderOptions)
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->onStartWindowRender(getWindowID());
        }
        return true;
    }
    void RenderTarget::onFinishRender(RenderOptions* renderOptions)
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->onFinishWindowRender(getWindowID());
        }
    }
}
