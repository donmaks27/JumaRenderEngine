// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_OpenGL_GLFW.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL) && defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)

#include <GLFW/glfw3.h>

namespace JumaRenderEngine
{
    WindowController_OpenGL_GLFW::~WindowController_OpenGL_GLFW()
    {
        clearGLFW();
    }

    bool WindowController_OpenGL_GLFW::initWindowController()
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

        glfwSetErrorCallback(WindowController_OpenGL_GLFW::GLFW_ErrorCallback);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 0);
        return true;
    }
    void WindowController_OpenGL_GLFW::GLFW_ErrorCallback(int errorCode, const char* errorMessage)
    {
        JUMA_RENDER_LOG(error, JSTR("GLFW error. Code: ") + TO_JSTR(errorCode) + JSTR(". ") + errorMessage);
    }

    void WindowController_OpenGL_GLFW::clearGLFW()
    {
        for (const auto& window : m_Windows)
        {
            destroyWindowGLFW(window.key, window.value);
        }
        m_Windows.clear();

        if (m_DefaultWindow != nullptr)
        {
            glfwDestroyWindow(m_DefaultWindow);
        }

        glfwTerminate();
    }

    bool WindowController_OpenGL_GLFW::createWindow(const window_id windowID, const WindowProperties& properties)
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

        if (m_DefaultWindow == nullptr)
        {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            m_DefaultWindow = glfwCreateWindow(1, 1, "", nullptr, nullptr);
            glfwMakeContextCurrent(m_DefaultWindow);
            initOpenGL();
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(properties.size.x), static_cast<int>(properties.size.y), *properties.title, nullptr, m_DefaultWindow
        );
        if (window == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create window ") + TO_JSTR(windowID));
            return false;
        }

        WindowData_OpenGL_GLFW& windowData = m_Windows[windowID];
        windowData.windowID = windowID;
        windowData.size = properties.size;
        windowData.windowGLFW = window;
        windowData.windowController = this;
        glfwSetWindowUserPointer(window, &windowData);
        glfwSetFramebufferSizeCallback(window, WindowController_OpenGL_GLFW::GLFW_FramebufferResizeCallback);

        const window_id prevActiveWindowID = getActiveWindowID();
        setActiveWindowID(windowID);
        glfwSwapInterval(1);
        setActiveWindowID(prevActiveWindowID);
        return true;
    }
    void WindowController_OpenGL_GLFW::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height)
    {
        const WindowData_OpenGL_GLFW* windowData = static_cast<WindowData_OpenGL_GLFW*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            windowData->windowController->onWindowResized(windowData->windowID, { math::max<uint32>(width, 0), math::max<uint32>(height, 0) });
        }
    }

    bool WindowController_OpenGL_GLFW::setActiveWindowInternal(const window_id windowID)
    {
        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData != nullptr)
        {
            glfwMakeContextCurrent(windowData->windowGLFW);
        }
        else
        {
            glfwMakeContextCurrent(m_DefaultWindow);
        }
        return true;
    }

    void WindowController_OpenGL_GLFW::destroyWindow(const window_id windowID)
    {
        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return;
        }

        destroyWindowGLFW(windowID, *windowData);
        m_Windows.remove(windowID);
    }
    void WindowController_OpenGL_GLFW::destroyWindowGLFW(const window_id windowID, const WindowData_OpenGL_GLFW& windowData)
    {
        glfwSetWindowUserPointer(windowData.windowGLFW, nullptr);
        glfwDestroyWindow(windowData.windowGLFW);
    }

    bool WindowController_OpenGL_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window ") + TO_JSTR(windowID));
            return false;
        }
        return glfwWindowShouldClose(windowData->windowGLFW) != GLFW_FALSE;
    }

    void WindowController_OpenGL_GLFW::onFinishWindowRender(const window_id windowID)
    {
        Super::onFinishWindowRender(windowID);

        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData != nullptr)
        {
            glfwSwapBuffers(windowData->windowGLFW);
        }
    }
    void WindowController_OpenGL_GLFW::onFinishRender()
    {
        Super::onFinishRender();

        glfwPollEvents();
    }
}

#endif
