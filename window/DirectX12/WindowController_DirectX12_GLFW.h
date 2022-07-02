// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "WindowController_DirectX12.h"

#include "jutils/jmap.h"

struct GLFWwindow;

namespace JumaRenderEngine
{
    class WindowController_DirectX12_GLFW;

    struct WindowData_DirectX12_GLFW : WindowData_DirectX12
    {
        GLFWwindow* windowGLFW = nullptr;

        WindowController_DirectX12_GLFW* windowController = nullptr;
    };

    class WindowController_DirectX12_GLFW final : public WindowController_DirectX12
    {
        using Super = WindowController_DirectX12;

    public:
        WindowController_DirectX12_GLFW() = default;
        virtual ~WindowController_DirectX12_GLFW() override;

        virtual void destroyWindow(window_id windowID) override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }
        virtual jarray<window_id> getWindowIDs() const override { return m_Windows.getKeys(); }

        virtual bool shouldCloseWindow(window_id windowID) const override;

        virtual void onFinishRender() override;

        virtual bool setWindowTitle(window_id windowID, const jstring& title) override;

    protected:

        virtual bool initWindowController() override;

        virtual WindowData* createWindowInternal(window_id windowID, const WindowProperties& properties) override;

        virtual WindowData* getWindowData(const window_id windowID) override { return m_Windows.find(windowID); }

    private:

        jmap<window_id, WindowData_DirectX12_GLFW> m_Windows;


        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);
        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);

        void clearGLFW();

        void clearWindowGLFW(window_id windowID, WindowData_DirectX12_GLFW& windowData);
    };
}

#endif
