// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/window/WindowController.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct IDXGISwapChain1;

namespace JumaRenderEngine
{
    struct WindowData_DirectX11 : WindowData
    {
        HWND windowHandler = nullptr;
        IDXGISwapChain1* swapchain = nullptr;
    };

    class WindowController_DirectX11 : public WindowController
    {
        using Super = WindowController;

    public:
        WindowController_DirectX11() = default;
        virtual ~WindowController_DirectX11() override;

        bool createWindowSwapchains();
        void clearWindowSwapchains();

        virtual void onFinishWindowRender(window_id windowID) override;

    protected:

        virtual bool initWindowController() override;

        void clearWindowDirectX11(window_id windowID, WindowData_DirectX11& windowData);

        bool createWindowSwapchain(window_id windowID, WindowData_DirectX11& windowData);

        virtual void onWindowResized(window_id windowID, const math::uvector2& newSize) override;

    private:

        bool m_TearingSupported = false;


        void clearDirectX11();

        void destroyWindowSwapchain(window_id windowID, WindowData_DirectX11& windowData);
    };
}

#endif
