// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"
#include "renderEngine/Vulkan/vulkanObjects/VulkanSwapchain.h"

namespace JumaRenderEngine
{
    void WindowController_Vulkan::destroyWindowVulkan(const window_id windowID, WindowData_Vulkan& windowData)
    {
        destroyWindowSwapchain(windowID, windowData);
        vkDestroySurfaceKHR(getRenderEngine<RenderEngine_Vulkan>()->getVulkanInstance(), windowData.vulkanSurface, nullptr);
    }

    bool WindowController_Vulkan::createWindowSwapchain(const window_id windowID, WindowData_Vulkan& windowData)
    {
        if (windowData.vulkanSwapchain == nullptr)
        {
            VulkanSwapchain* swapchain = getRenderEngine()->createObject<VulkanSwapchain>();
            if (!swapchain->init(windowID))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan swapchain for window ") + TO_JSTR(windowID));
                delete swapchain;
                return false;
            }
            windowData.vulkanSwapchain = swapchain;
        }
        return true;
    }
    void WindowController_Vulkan::destroyWindowSwapchain(const window_id windowID, WindowData_Vulkan& windowData)
    {
        delete windowData.vulkanSwapchain;
    }

    bool WindowController_Vulkan::createWindowSwapchains()
    {
        for (const auto& window : getVulkanWindowsDataPtr())
        {
            if (!createWindowSwapchain(window.key, *window.value))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan swapchain"));
                return false;
            }
        }
        return true;
    }
}

#endif
