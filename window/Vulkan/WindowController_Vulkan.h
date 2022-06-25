// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/window/WindowController.h"

#include <vulkan/vulkan_core.h>

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
        using Super = WindowController;

    public:
        WindowController_Vulkan() = default;
        virtual ~WindowController_Vulkan() override;

        virtual jarray<const char*> getVulkanInstanceExtensions() const = 0;

        bool createWindowSwapchains();
        void clearWindowSwapchains();

    protected:

        void clearWindowVulkan(window_id windowID, WindowData_Vulkan& windowData);

        bool createWindowSwapchain(window_id windowID, WindowData_Vulkan& windowData);

    private:

        void clearVulkan();

        void destroyWindowSwapchain(window_id windowID, WindowData_Vulkan& windowData);
    };
}

#endif
