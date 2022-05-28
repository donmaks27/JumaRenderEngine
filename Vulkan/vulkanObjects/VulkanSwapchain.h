// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngineContextObject.h"

#include <vulkan/vulkan_core.h>

#include "jutils/jarray.h"
#include "jutils/math/vector2.h"
#include "renderEngine/window/window_id.h"

namespace JumaRenderEngine
{
    class WindowController_Vulkan;

    class VulkanSwapchain : public RenderEngineContextObjectBase
    {
        friend WindowController_Vulkan;

    public:
        VulkanSwapchain() = default;
        virtual ~VulkanSwapchain() override;

        VkSwapchainKHR get() const { return m_Swapchain; }
        window_id getWindowID() const { return m_WindowID; }

        const jarray<VkImage>& getImages() const { return m_SwapchainImages; }
        VkFormat getImagesFormat() const { return m_SwapchainImagesFormat; }
        const math::uvector2& getImagesSize() const { return m_SwapchainImagesSize; }

        VkSemaphore getRenderAvailableSemaphore() const { return m_RenderAvailableSemaphore; }
        int8 getAcquiredImageIndex() const { return m_AcquiredSwapchainImageIndex; }

        void markAsNeedToRecreate() { m_NeedToRecreate = true; }
        bool updateSwapchain();

        bool acquireNextImage(bool& availableForRender);

    private:

        window_id m_WindowID = window_id_INVALID;

        VkSwapchainKHR m_Swapchain = nullptr;
        bool m_NeedToRecreate = false;

        jarray<VkImage> m_SwapchainImages;
        VkFormat m_SwapchainImagesFormat = VK_FORMAT_UNDEFINED;
        math::uvector2 m_SwapchainImagesSize = { 0, 0 };

        VkSemaphore m_RenderAvailableSemaphore = nullptr;
        int8 m_AcquiredSwapchainImageIndex = -1;


        bool init(window_id windowID);
        bool createSwapchain(VkSwapchainKHR oldSwapchain);

        void clearVulkan();
    };
}

#endif
