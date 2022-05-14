// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderTarget.h"

#include "RenderEngine.h"

namespace JumaRenderEngine
{
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
            return false;
        }
        return true;
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

    void RenderTarget::onStartRender()
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->onStartWindowRender(getWindowID());
        }
    }
    void RenderTarget::onFinishRender()
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->onFinishWindowRender(getWindowID());
        }
    }
}
