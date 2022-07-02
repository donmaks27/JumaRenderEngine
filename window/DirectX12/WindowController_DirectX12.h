// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/window/WindowController.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace JumaRenderEngine
{
    class DirectX12Swapchain;

    struct WindowData_DirectX12 : WindowData
    {
        HWND windowHandler = nullptr;

        DirectX12Swapchain* swapchain = nullptr;
    };

    class WindowController_DirectX12 : public WindowController
    {
        using Super = WindowController;

    public:
        WindowController_DirectX12() = default;
        virtual ~WindowController_DirectX12() override;

        bool isTearingSupported() const { return m_TearingSupported; }

        bool createWindowSwapchains();
        void clearWindowSwapchains();

    protected:

        virtual bool initWindowController() override;

        void clearWindowDirectX(window_id windowID, WindowData_DirectX12& windowData);

        bool createWindowSwapchain(window_id windowID, WindowData_DirectX12& windowData);

    private:

        bool m_TearingSupported = false;


        void clearDirectX();

        void destroyWindowSwapchain(window_id windowID, WindowData_DirectX12& windowData);
    };
}

#endif
