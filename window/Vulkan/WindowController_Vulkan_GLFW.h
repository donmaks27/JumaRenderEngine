// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN) && defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)

#include "WindowController_Vulkan.h"

#include "jutils/jmap.h"

struct GLFWwindow;

namespace JumaRenderEngine
{
    class WindowController_Vulkan_GLFW;

    struct WindowData_Vulkan_GLFW : WindowData_Vulkan
    {
        GLFWwindow* windowGLFW = nullptr;

        WindowController_Vulkan_GLFW* windowController = nullptr;
    };

    class WindowController_Vulkan_GLFW final : public WindowController_Vulkan
    {
        using Super = WindowController_Vulkan;

    public:
        WindowController_Vulkan_GLFW() = default;
        virtual ~WindowController_Vulkan_GLFW() override;

        virtual jarray<const char*> getVulkanInstanceExtensions() const override;

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

        jmap<window_id, WindowData_Vulkan_GLFW> m_Windows;

        jmap<window_id, math::uvector2> m_ChangedWindowSizes;


        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);
        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);

        void clearGLFW();

        void clearWindowGLFW(window_id windowID, WindowData_Vulkan_GLFW& windowData);
    };
}

#endif
