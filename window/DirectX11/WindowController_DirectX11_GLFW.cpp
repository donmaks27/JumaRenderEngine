// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_DirectX11_GLFW.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11) && defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)

#include <GLFW/glfw3.h>

namespace JumaRenderEngine
{
    WindowController_DirectX11_GLFW::~WindowController_DirectX11_GLFW()
    {
        clearGLFW();
    }

    bool WindowController_DirectX11_GLFW::initWindowController()
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
            JUMA_RENDER_LOG(error, jstring(JSTR("Failed to initialize GLFW lib: ")) + errorStr);
#endif
            return false;
        }

        glfwSetErrorCallback(WindowController_DirectX11_GLFW::GLFW_ErrorCallback);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return true;
    }
    void WindowController_DirectX11_GLFW::GLFW_ErrorCallback(const int errorCode, const char* errorMessage)
    {
        JUMA_RENDER_LOG(error, JSTR("GLFW error. Code: ") + TO_JSTR(errorCode) + JSTR(". ") + errorMessage);
    }

    void WindowController_DirectX11_GLFW::clearGLFW()
    {
        if (!m_Windows.isEmpty())
        {
            for (auto& window : m_Windows)
            {
                destroyWindowGLFW(window.key, window.value);
            }
            m_Windows.clear();
        }

        glfwTerminate();
    }

    bool WindowController_DirectX11_GLFW::createWindow(const window_id windowID, const WindowProperties& properties)
    {
        if (windowID == window_id_INVALID)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid window ID"));
            return false;
        }
        if (m_Windows.contains(windowID))
        {
            JUMA_RENDER_LOG(error, JSTR("Window ") + TO_JSTR(windowID) + JSTR(" already created"));
            return false;
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(properties.size.x), static_cast<int>(properties.size.y), *properties.title, nullptr, nullptr
        );
        if (window == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create window ") + TO_JSTR(windowID));
            return false;
        }

        WindowData_DirectX11_GLFW& windowData = m_Windows[windowID];
        windowData.windowID = windowID;
        windowData.size = properties.size;
        windowData.windowGLFW = window;
        windowData.windowController = this;
        glfwSetWindowUserPointer(window, &windowData);
        glfwSetFramebufferSizeCallback(window, WindowController_DirectX11_GLFW::GLFW_FramebufferResizeCallback);
        return true;
    }
    void WindowController_DirectX11_GLFW::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, const int width, const int height)
    {
        const WindowData_DirectX11_GLFW* windowData = static_cast<WindowData_DirectX11_GLFW*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            windowData->windowController->onWindowResized(windowData->windowID, { math::max<uint32>(width, 0), math::max<uint32>(height, 0) });
        }
    }

    void WindowController_DirectX11_GLFW::destroyWindow(const window_id windowID)
    {
        WindowData_DirectX11_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return;
        }

        destroyWindowGLFW(windowID, *windowData);
        m_Windows.remove(windowID);
    }
    void WindowController_DirectX11_GLFW::destroyWindowGLFW(const window_id windowID, WindowData_DirectX11_GLFW& windowData)
    {
        glfwSetWindowUserPointer(windowData.windowGLFW, nullptr);
        glfwDestroyWindow(windowData.windowGLFW);
        windowData.windowGLFW = nullptr;
        windowData.windowController = nullptr;
    }

    bool WindowController_DirectX11_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_DirectX11_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return false;
        }
        return glfwWindowShouldClose(windowData->windowGLFW) != GLFW_FALSE;
    }

    void WindowController_DirectX11_GLFW::onFinishRender()
    {
        Super::onFinishRender();

        glfwPollEvents();
    }

    bool WindowController_DirectX11_GLFW::setWindowTitle(const window_id windowID, const jstring& title)
    {
        const WindowData_DirectX11_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return false;
        }

        glfwSetWindowTitle(windowData->windowGLFW, *title);
        return true;
    }
}

#endif
