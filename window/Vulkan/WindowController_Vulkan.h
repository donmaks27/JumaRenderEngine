// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/window/WindowController.h"

#include <vulkan/vulkan_core.h>

#include "jutils/jarray.h"
#include "jutils/jmap.h"

namespace JumaRenderEngine
{
    class VulkanSwapchain;

    struct WindowData_Vulkan : WindowData
    {
        VkSurfaceKHR vulkanSurface = nullptr;
        VulkanSwapchain* vulkanSwapchain = nullptr;
    };

    class WindowController_Vulkan : public WindowController
    {
    public:
        WindowController_Vulkan() = default;
        virtual ~WindowController_Vulkan() override = default;

        virtual jarray<const char*> getVulkanInstanceExtensions() const = 0;

        virtual jmap<window_id, const WindowData_Vulkan*> getVulkanWindowsData() const = 0;

        bool createWindowSwapchains();

    protected:
        
        void destroyWindowVulkan(window_id windowID, WindowData_Vulkan& windowData);

        bool createWindowSwapchain(window_id windowID, WindowData_Vulkan& windowData);
        void destroyWindowSwapchain(window_id windowID, WindowData_Vulkan& windowData);

        virtual jmap<window_id, WindowData_Vulkan*> getVulkanWindowsDataPtr() = 0;
    };
}

#endif
