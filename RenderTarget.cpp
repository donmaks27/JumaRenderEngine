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
        WindowController* windowController = getRenderEngine()->getWindowController();

        math::uvector2 windowSize;
        if (!windowController->getActualWindowSize(windowID, windowSize))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get size of window {}"), windowID);
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

        m_Invalid = false;
        windowController->OnWindowPropertiesChanged.bind(this, &RenderTarget::onWindowPropertiesChanged);
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

        m_Invalid = false;
        return true;
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

    bool RenderTarget::update()
    {
        if (m_Invalid)
        {
            if (!recreateRenderTarget())
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to recreate render target"));
                return false;
            }
            m_Invalid = false;
        }
        return true;
    }

    void RenderTarget::onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData)
    {
        if (windowData->windowID == getWindowID())
        {
            if ((windowData->properties.size != m_Size) || (windowData->properties.samples != m_TextureSamples))
            {
                m_Size = windowData->properties.size;
                m_TextureSamples = windowData->properties.samples;
                invalidate();
            }
        }
    }

    bool RenderTarget::onStartRender(RenderOptions* renderOptions)
    {
        if (!update())
        {
            return false;
        }
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
