// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VulkanSwapchain.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngine.h"
#include "renderEngine/Vulkan/RenderEngine_Vulkan.h"
#include "renderEngine/window/Vulkan/WindowController_Vulkan.h"

namespace JumaRenderEngine
{
    VulkanSwapchain::~VulkanSwapchain()
    {
        clearVulkan();
    }

    bool VulkanSwapchain::init(const window_id windowID)
    {
        m_WindowID = windowID;
        if (!createSwapchain(nullptr))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan swapchain"));
            clearVulkan();
            return false;
        }
        if (!createSyncObjects())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan sync objects"));
            clearVulkan();
            return false;
        }
        return true;
    }
    bool VulkanSwapchain::createSwapchain(VkSwapchainKHR oldSwapchain)
    {
        constexpr bool tripleBuffering = false;
        constexpr uint8 defaultImageCount = tripleBuffering ? 3 : 2;

        const RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const WindowData_Vulkan* windowData = reinterpret_cast<const WindowData_Vulkan*>(renderEngine->getWindowController()->findWindowData(m_WindowID));
        VkPhysicalDevice physicalDevice = renderEngine->getPhysicalDevice();
        VkDevice device = renderEngine->getDevice();

        uint32 surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, windowData->vulkanSurface, &surfaceFormatCount, nullptr);
        if (surfaceFormatCount == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("There is no vulkan surface formats for window ") + TO_JSTR(m_WindowID));
            return false;
        }
        jarray<VkSurfaceFormatKHR> surfaceFormats(static_cast<int32>(surfaceFormatCount));
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, windowData->vulkanSurface, &surfaceFormatCount, surfaceFormats.getData());
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, windowData->vulkanSurface, &surfaceCapabilities);

        uint32 imageCount = math::clamp(
            defaultImageCount, 
            surfaceCapabilities.minImageCount, 
            surfaceCapabilities.maxImageCount > 0 ? surfaceCapabilities.maxImageCount : defaultImageCount
        );
        const VkExtent2D swapchainSize = {
		    math::clamp(windowData->size.x, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
		    math::clamp(windowData->size.y, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
	    };
        VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
        for (const auto& format : surfaceFormats)
        {
            if ((format.format == VK_FORMAT_B8G8R8A8_SRGB) && (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
            {
                surfaceFormat = format;
                break;
            }
        }

        VkSwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	    swapchainInfo.surface = windowData->vulkanSurface;
	    swapchainInfo.minImageCount = imageCount;
	    swapchainInfo.imageFormat = surfaceFormat.format;
	    swapchainInfo.imageColorSpace = surfaceFormat.colorSpace;
	    swapchainInfo.imageExtent = swapchainSize;
	    swapchainInfo.imageArrayLayers = 1;
	    swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = nullptr;
        swapchainInfo.preTransform = surfaceCapabilities.currentTransform;
	    swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	    swapchainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	    swapchainInfo.clipped = VK_TRUE;
	    swapchainInfo.oldSwapchain = oldSwapchain;
        const VkResult result = vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &m_Swapchain);
        if (oldSwapchain != nullptr)
        {
            vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
        }
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan swapchain"));
            return false;
        }

        vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr);
        m_SwapchainImages.resize(static_cast<int32>(imageCount));
        vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, m_SwapchainImages.getData());

        m_SwapchainImagesFormat = surfaceFormat.format;
        m_SwapchainImagesSize = { swapchainSize.width, swapchainSize.height };
        return true;
    }
    bool VulkanSwapchain::createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        const VkResult result = vkCreateSemaphore(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), &semaphoreInfo, nullptr, &m_RenderAvailableSemaphore);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create RenderAvailableSemaphore"));
            return false;
        }
        return true;
    }

    void VulkanSwapchain::clearVulkan()
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        if (m_RenderAvailableSemaphore != nullptr)
        {
            vkDestroySemaphore(device, m_RenderAvailableSemaphore, nullptr);
            m_RenderAvailableSemaphore = nullptr;
        }
        m_SwapchainImages.clear();
        if (m_Swapchain != nullptr)
        {
            vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
            m_Swapchain = nullptr;
        }

        m_WindowID = window_id_INVALID;
        m_NeedToRecreate = false;
        m_SwapchainImagesFormat = VK_FORMAT_UNDEFINED;
        m_SwapchainImagesSize = { 0, 0 };
        m_AcquiredSwapchainImageIndex = -1;
    }

    bool VulkanSwapchain::refreshSwapchain()
    {
        if (!m_NeedToRecreate)
        {
            return true;
        }

        if (!createSwapchain(m_Swapchain))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to recreate vulkan swapchain"));
            return false;
        }
        m_NeedToRecreate = false;
        return true;
    }

    bool VulkanSwapchain::acquireNextImage(bool& availableForRender)
    {
        availableForRender = false;
        if (!m_NeedToRecreate)
        {
            uint32 renderImageIndex = 0;
            const VkResult result = vkAcquireNextImageKHR(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), m_Swapchain, UINT64_MAX, m_RenderAvailableSemaphore, nullptr, &renderImageIndex);
            if ((result != VK_SUCCESS) && (result != VK_SUBOPTIMAL_KHR))
            {
                if (result != VK_ERROR_OUT_OF_DATE_KHR)
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to acquire swapchain image"));
                    return false;
                }
                
                m_NeedToRecreate = true;
            }
            else
            {
                availableForRender = true;
                m_AcquiredSwapchainImageIndex = static_cast<int8>(renderImageIndex);
            }
        }
        return true;
    }
}

#endif
