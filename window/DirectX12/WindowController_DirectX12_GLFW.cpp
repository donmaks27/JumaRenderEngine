// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_DirectX12_GLFW.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace JumaRenderEngine
{
    WindowController_DirectX12_GLFW::~WindowController_DirectX12_GLFW()
    {
        clearGLFW();
    }

    bool WindowController_DirectX12_GLFW::initWindowController()
    {
        if (!Super::initWindowController())
        {
            return false;
        }

        if (glfwInit() == GLFW_FALSE)
        {
#ifndef JUTILS_LOG_DISABLED
            const char* errorStr = nullptr;
            glfwGetError(&errorStr);
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize GLFW lib: {}"), errorStr);
#endif
            return false;
        }

        glfwSetErrorCallback(WindowController_DirectX12_GLFW::GLFW_ErrorCallback);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return true;
    }
    void WindowController_DirectX12_GLFW::GLFW_ErrorCallback(const int errorCode, const char* errorMessage)
    {
        JUMA_RENDER_LOG(error, JSTR("GLFW error. Code: {}. {}"), errorCode, errorMessage);
    }

    void WindowController_DirectX12_GLFW::clearGLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows)
            {
                clearWindowGLFW(window.key, window.value);
            }
            m_Windows.clear();
        }

        glfwTerminate();
    }

    WindowData* WindowController_DirectX12_GLFW::createWindowInternal(const window_id windowID, const WindowProperties& properties)
    {
        if (windowID == window_id_INVALID)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid window ID"));
            return nullptr;
        }
        if (m_Windows.contains(windowID))
        {
            JUMA_RENDER_LOG(error, JSTR("Window {} already created"), windowID);
            return nullptr;
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(properties.size.x), static_cast<int>(properties.size.y), *properties.title, nullptr, nullptr
        );
        if (window == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create window {}"), windowID);
            return nullptr;
        }

        WindowData_DirectX12_GLFW& windowData = m_Windows[windowID];
        windowData.windowHandler = glfwGetWin32Window(window);
        windowData.windowGLFW = window;
        windowData.windowController = this;
        glfwSetWindowUserPointer(window, &windowData);
        glfwSetFramebufferSizeCallback(window, WindowController_DirectX12_GLFW::GLFW_FramebufferResizeCallback);
        glfwSetWindowIconifyCallback(window, WindowController_DirectX12_GLFW::GLFW_WindowMinimizationCallback);

        if (!createWindowSwapchain(windowID, windowData))
        {
            clearWindowGLFW(windowID, windowData);
            m_Windows.remove(windowID);
            return nullptr;
        }
        return &windowData;
    }
    void WindowController_DirectX12_GLFW::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, const int width, const int height)
    {
        const WindowData_DirectX12_GLFW* windowData = static_cast<WindowData_DirectX12_GLFW*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            windowData->windowController->updateWindowSize(windowData->windowID, { math::max<uint32>(width, 0), math::max<uint32>(height, 0) });
        }
    }
    void WindowController_DirectX12_GLFW::GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized)
    {
        const WindowData_DirectX12_GLFW* windowData = static_cast<WindowData_DirectX12_GLFW*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            windowData->windowController->updateWindowMinimization(windowData->windowID, minimized == GLFW_TRUE);
        }
    }

    void WindowController_DirectX12_GLFW::destroyWindow(const window_id windowID)
    {
        WindowData_DirectX12_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window {}"), windowID);
            return;
        }

        clearWindowGLFW(windowID, *windowData);
        m_Windows.remove(windowID);
    }
    void WindowController_DirectX12_GLFW::clearWindowGLFW(const window_id windowID, WindowData_DirectX12_GLFW& windowData)
    {
        clearWindowDirectX(windowID, windowData);

        glfwSetWindowUserPointer(windowData.windowGLFW, nullptr);
        glfwDestroyWindow(windowData.windowGLFW);
        windowData.windowGLFW = nullptr;
        windowData.windowController = nullptr;
    }

    bool WindowController_DirectX12_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_DirectX12_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window {}"), windowID);
            return false;
        }
        return glfwWindowShouldClose(windowData->windowGLFW) != GLFW_FALSE;
    }

    void WindowController_DirectX12_GLFW::updateWindows()
    {
        glfwPollEvents();

        Super::updateWindows();
    }

    bool WindowController_DirectX12_GLFW::setWindowTitle(window_id windowID, const jstring& title)
    {
        const WindowData_DirectX12_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window {}"), windowID);
            return false;
        }

        glfwSetWindowTitle(windowData->windowGLFW, *title);
        return true;
    }
}

#endif
