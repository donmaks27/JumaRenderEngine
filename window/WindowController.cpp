// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController.h"

#include "renderEngine/RenderEngine.h"
#include "renderEngine/RenderTarget.h"

namespace JumaRenderEngine
{
    bool WindowController::createWindow(const window_id windowID, const WindowProperties& properties)
    {
        WindowData* windowData = createWindowInternal(windowID, properties);
        if (windowData == nullptr)
        {
            return false;
        }

        windowData->windowID = windowID;
        windowData->properties = properties;
        if (!createRenderTarget(windowID, *windowData))
        {
            destroyWindow(windowID);
            return false;
        }

        m_WindowsCount++;
        return true;
    }

    void WindowController::clearWindow(const window_id windowID, WindowData& windowData)
    {
        m_WindowsCount--;
        if (windowData.minimized)
        {
            m_MinimizedWindowsCount--;
        }

        clearRenderTarget(windowID, windowData);
    }

    bool WindowController::createRenderTarget(const window_id windowID, WindowData& windowData)
    {
        RenderEngine* renderEngine = getRenderEngine();
        if (!renderEngine->isValid())
        {
            return true;
        }

        if (windowData.windowRenderTarget != nullptr)
        {
            return true;
        }
        RenderTarget* renderTarget = renderEngine->createWindowRenderTarget(windowID, windowData.properties.samples);
        if (renderTarget == nullptr)
        {
            return false;
        }
        windowData.windowRenderTarget = renderTarget;
        return true;
    }
    void WindowController::clearRenderTarget(const window_id windowID, WindowData& windowData)
    {
        if (windowData.windowRenderTarget != nullptr)
        {
            delete windowData.windowRenderTarget;
            windowData.windowRenderTarget = nullptr;
        }
    }

    bool WindowController::createRenderTargets()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createRenderTarget(windowID, *getWindowData(windowID)))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX11 render target for window ") + TO_JSTR(windowID));
                return false;
            }
        }
        return true;
    }
    void WindowController::clearRenderTargets()
    {
        for (const auto& windowID : getWindowIDs())
        {
            clearRenderTarget(windowID, *getWindowData(windowID));
        }
    }

    bool WindowController::getActualWindowSize(const window_id windowID, math::uvector2& outSize) const
    {
        const WindowData* windowData = findWindowData(windowID);
        if (windowData == nullptr)
        {
            return false;
        }
        outSize = windowData->properties.size;
        return true;
    }

    void WindowController::updateWindowSize(const window_id windowID, const math::uvector2& size)
    {
        m_ChangedWindowSizes.add(windowID, size);
    }
    void WindowController::updateWindows()
    {
        if (!m_ChangedWindowSizes.isEmpty())
        {
            for (const auto& changedWindowSize : m_ChangedWindowSizes)
            {
                WindowData* windowData = getWindowData(changedWindowSize.key);
                if (windowData != nullptr)
                {
                    windowData->properties.size = changedWindowSize.value;
                    onWindowResized(windowData);
                    OnWindowPropertiesChanged.call(this, windowData);
                }
            }
            m_ChangedWindowSizes.clear();
        }
    }

    void WindowController::updateWindowMinimization(const window_id windowID, const bool minimized)
    {
        WindowData* windowData = getWindowData(windowID);
        if (windowData->minimized != minimized)
        {
            windowData->minimized = minimized;
            onWindowMinimizationChanged(windowData);
        }
    }
    void WindowController::onWindowMinimizationChanged(WindowData* windowData)
    {
        if (windowData->minimized)
        {
            m_MinimizedWindowsCount++;
        }
        else
        {
            m_MinimizedWindowsCount--;
        }
    }
    bool WindowController::isWindowMinimized(const window_id windowID) const
    {
        const WindowData* windowData = findWindowData(windowID);
        return (windowData != nullptr) && windowData->minimized;
    }
}
