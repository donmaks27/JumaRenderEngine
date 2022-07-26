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
        const WindowData* windowData = getRenderEngine()->getWindowController()->findWindowData(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("There is not window ") + TO_JSTR(windowID));
            return false;
        }

        m_WindowID = windowID;
        m_TextureSamples = samples;
        m_Size = windowData->properties.size;
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

    bool RenderTarget::initInternal()
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->OnWindowPropertiesChanged.bind(this, &RenderTarget::onWindowPropertiesChanged);
        }
        return true;
    }

    void RenderTarget::onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData)
    {
        if ((windowData != nullptr) && (windowData->windowID == m_WindowID))
        {
            if ((m_Size != windowData->properties.size) || (m_TextureSamples != windowData->properties.samples))
            {
                const math::vector2 prevSize = m_Size;
                const TextureSamples prevSamples = m_TextureSamples;
                m_Size = windowData->properties.size;
                m_TextureSamples = windowData->properties.samples;
                onPropertiesChanged(prevSize, prevSamples);
            }
        }
    }

    void RenderTarget::clearData()
    {
        if (isWindowRenderTarget())
        {
            getRenderEngine()->getWindowController()->OnWindowPropertiesChanged.unbind(this, &RenderTarget::onWindowPropertiesChanged);
        }

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
