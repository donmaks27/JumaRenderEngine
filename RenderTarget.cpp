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
        math::uvector2 windowSize;
        if (!getRenderEngine()->getWindowController()->getActualWindowSize(windowID, windowSize))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get size of window ") + TO_JSTR(windowID));
            return false;
        }

        m_WindowID = windowID;
        m_TextureSamples = samples;
        m_Size = windowSize;
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

    void RenderTarget::changeProperties(const math::uvector2& size, const TextureSamples samples)
    {
        if ((m_Size != size) || (m_TextureSamples != samples))
        {
            const math::uvector2 prevSize = m_Size;
            const TextureSamples prevSamples = m_TextureSamples;
            m_Size = size;
            m_TextureSamples = samples;
            onPropertiesChanged(prevSize, prevSamples);
        }
    }

    void RenderTarget::clearData()
    {
        m_WindowID = window_id_INVALID;
        m_TextureSamples = TextureSamples::X1;
        m_Size = { 0, 0 };
        m_Format = TextureFormat::RGBA8;
    }

    bool RenderTarget::onStartRender(RenderOptions* renderOptions)
    {
        if (isWindowRenderTarget())
        {
            return getRenderEngine()->getWindowController()->onStartWindowRender(getWindowID());
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
