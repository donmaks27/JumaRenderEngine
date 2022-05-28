// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"
#include "renderEngine/Vulkan/vulkanObjects/VulkanSwapchain.h"

namespace JumaRenderEngine
{
    WindowController_Vulkan::~WindowController_Vulkan()
    {
        clearVulkan();
    }

    void WindowController_Vulkan::destroyWindowVulkan(const window_id windowID, WindowData_Vulkan& windowData)
    {
        destroyWindowSwapchain(windowID, windowData);

        vkDestroySurfaceKHR(getRenderEngine<RenderEngine_Vulkan>()->getVulkanInstance(), windowData.vulkanSurface, nullptr);
        windowData.vulkanSurface = nullptr;
    }

    void WindowController_Vulkan::clearVulkan()
    {
    }

    bool WindowController_Vulkan::createWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createWindowSwapchain(windowID, *getWindowData<WindowData_Vulkan>(windowID)))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan swapchain"));
                return false;
            }
        }
        return true;
    }
    void WindowController_Vulkan::clearWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            destroyWindowSwapchain(windowID, *getWindowData<WindowData_Vulkan>(windowID));
        }
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
        if (windowData.vulkanSwapchain != nullptr)
        {
            delete windowData.vulkanSwapchain;
            windowData.vulkanSwapchain = nullptr;
        }
    }
}

#endif
