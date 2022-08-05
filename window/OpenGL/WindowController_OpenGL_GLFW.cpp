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
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize GLFW lib: {}"), errorStr);
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
        JUMA_RENDER_LOG(error, JSTR("GLFW error. Code: {}. {}"), errorCode, errorMessage);
    }

    void WindowController_OpenGL_GLFW::clearGLFW()
    {
        for (auto& window : m_Windows)
        {
            clearWindowGLFW(window.key, window.value);
        }
        m_Windows.clear();

        if (m_DefaultWindow != nullptr)
        {
            glfwDestroyWindow(m_DefaultWindow);
        }

        glfwTerminate();
    }

    WindowData* WindowController_OpenGL_GLFW::createWindowInternal(const window_id windowID, const WindowProperties& properties)
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

        if (m_DefaultWindow == nullptr)
        {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            m_DefaultWindow = glfwCreateWindow(1, 1, "", nullptr, nullptr);
            glfwMakeContextCurrent(m_DefaultWindow);
            initOpenGL();
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        GLFWwindow* window = glfwCreateWindow(
            static_cast<int>(properties.size.x), static_cast<int>(properties.size.y), *properties.title, nullptr, m_DefaultWindow
        );
        if (window == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create window {}"), windowID);
            return nullptr;
        }

        WindowData_OpenGL_GLFW& windowData = m_Windows[windowID];
        windowData.windowGLFW = window;
        windowData.windowController = this;
        glfwSetWindowUserPointer(window, &windowData);
        glfwSetFramebufferSizeCallback(window, WindowController_OpenGL_GLFW::GLFW_FramebufferResizeCallback);
        glfwSetWindowIconifyCallback(window, WindowController_OpenGL_GLFW::GLFW_WindowMinimizationCallback);

        const window_id prevActiveWindowID = getActiveWindowID();
        setActiveWindowID(windowID);
        glfwSwapInterval(1);
        setActiveWindowID(prevActiveWindowID);
        return &windowData;
    }
    void WindowController_OpenGL_GLFW::GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height)
    {
        const WindowData_OpenGL_GLFW* windowData = static_cast<WindowData_OpenGL_GLFW*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            windowData->windowController->updateWindowSize(windowData->windowID, { math::max<uint32>(width, 0), math::max<uint32>(height, 0) });
        }
    }
    void WindowController_OpenGL_GLFW::GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, const int minimized)
    {
        const WindowData_OpenGL_GLFW* windowData = static_cast<WindowData_OpenGL_GLFW*>(glfwGetWindowUserPointer(windowGLFW));
        if (windowData != nullptr)
        {
            windowData->windowController->updateWindowMinimization(windowData->windowID, minimized == GLFW_TRUE);
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
        WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData != nullptr)
        {
            clearWindowGLFW(windowID, *windowData);
            m_Windows.remove(windowID);
        }
    }
    void WindowController_OpenGL_GLFW::clearWindowGLFW(const window_id windowID, WindowData_OpenGL_GLFW& windowData)
    {
        clearWindow(windowID, windowData);

        glfwSetWindowUserPointer(windowData.windowGLFW, nullptr);
        glfwDestroyWindow(windowData.windowGLFW);
        windowData.windowGLFW = nullptr;
        windowData.windowController = nullptr;
    }

    bool WindowController_OpenGL_GLFW::shouldCloseWindow(const window_id windowID) const
    {
        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(warning, JSTR("Can't find window {}"), windowID);
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
    void WindowController_OpenGL_GLFW::updateWindows()
    {
        glfwPollEvents();

        Super::updateWindows();
    }

    bool WindowController_OpenGL_GLFW::setWindowTitle(const window_id windowID, const jstring& title)
    {
        const WindowData_OpenGL_GLFW* windowData = m_Windows.find(windowID);
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
