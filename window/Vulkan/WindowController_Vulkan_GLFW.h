// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN) && defined(JUMARENDERENGINE_INCLUDE_LIB_GLFW)

#include "WindowController_Vulkan.h"

struct GLFWwindow;

namespace JumaRenderEngine
{
    class WindowController_Vulkan_GLFW;

    struct WindowData_Vulkan_GLFW : WindowData_Vulkan
    {
        GLFWwindow* windowGLFW = nullptr;

        WindowController_Vulkan_GLFW* windowController = nullptr;
    };

    class WindowController_Vulkan_GLFW : public WindowController_Vulkan
    {
        using Super = WindowController_Vulkan;

    public:
        WindowController_Vulkan_GLFW() = default;
        virtual ~WindowController_Vulkan_GLFW() override;

        virtual jarray<const char*> getVulkanInstanceExtensions() const override;

        virtual bool createWindow(window_id windowID, const WindowProperties& properties) override;
        virtual void destroyWindow(window_id windowID) override;

        virtual const WindowData* findWindowData(const window_id windowID) const override { return m_Windows.find(windowID); }
        virtual jmap<window_id, const WindowData_Vulkan*> getVulkanWindowsData() const override;

        virtual bool shouldCloseWindow(window_id windowID) const override;

    protected:

        virtual bool initWindowController() override;
        virtual void clearWindowController() override { clearGLFW(); }

        virtual jmap<window_id, WindowData_Vulkan*> getVulkanWindowsDataPtr() override;
        virtual WindowData* findWindowDataPtr(const window_id windowID) override { return m_Windows.find(windowID); }

    private:

        jmap<window_id, WindowData_Vulkan_GLFW> m_Windows;

        
        static void GLFW_ErrorCallback(int errorCode, const char* errorMessage);
        static void GLFW_FramebufferResizeCallback(GLFWwindow* windowGLFW, int width, int height);

        void clearGLFW();

        void destroyWindowGLFW(window_id windowID, WindowData_Vulkan_GLFW& windowData);
    };
}

#endif
