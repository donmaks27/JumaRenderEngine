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
        return true;
    }

    void WindowController::clearWindow(const window_id windowID, WindowData& windowData)
    {
        clearRenderTarget(windowID, windowData);
    }

    void WindowController::onWindowResized(const window_id windowID, const math::uvector2& newSize)
    {
        WindowData* windowData = getWindowData(windowID);
        windowData->properties.size = newSize;
        OnWindowPropertiesChanged.call(this, windowData);
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
}
