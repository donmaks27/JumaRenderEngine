﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL) && defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)

#include "WindowController_OpenGL.h"

struct GLFWwindow;

namespace JumaRenderEngine
{
    class WindowController_OpenGL_GLFW;

    struct WindowData_OpenGL_GLFW : WindowData_OpenGL
    {
        GLFWwindow* windowGLFW = nullptr;

        WindowController_OpenGL_GLFW* windowController = nullptr;
    };

    class WindowController_OpenGL_GLFW final : public WindowController_OpenGL
    {
        using Super = WindowController_OpenGL;

    public:
        WindowController_OpenGL_GLFW() = default;
        virtual ~WindowController_OpenGL_GLFW() override;

        virtual void destroyWindow(window_id windowID) override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }
        virtual jarray<window_id> getWindowIDs() const override { return m_Windows.getKeys(); }

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void onFinishWindowRender(window_id windowID) override;
        virtual void updateWindows() override;

        virtual bool setWindowTitle(window_id windowID, const jstring& title) override;

    protected:

        virtual bool initWindowController() override;

        virtual WindowData* createWindowInternal(window_id windowID, const WindowProperties& properties) override;
        
        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

        virtual bool setActiveWindowInternal(window_id windowID) override;

    private:

        GLFWwindow* m_DefaultWindow = nullptr;
        jmap<window_id, WindowData_OpenGL_GLFW> m_Windows;


        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);
        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);
        static void GLFW_WindowMinimizationCallback(GLFWwindow* windowGLFW, int minimized);

        void clearGLFW();

        void clearWindowGLFW(window_id windowID, WindowData_OpenGL_GLFW& windowData);
    };
}

#endif
