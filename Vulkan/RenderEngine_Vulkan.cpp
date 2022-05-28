// Copyright 2022 Leonov Maksim. All Rights Reserved.

#define VMA_IMPLEMENTATION

#include "RenderEngine_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderPipeline_Vulkan.h"
#include "RenderTarget_Vulkan.h"
#include "renderEngine/window/Vulkan/WindowController_Vulkan.h"
#include "renderEngine/window/Vulkan/WindowControllerInfo_Vulkan.h"
#include "vulkanObjects/VulkanCommandPool.h"

#ifdef JDEBUG
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) 
    {
        func(instance, debugMessenger, pAllocator);
    }
}
#endif

namespace JumaRenderEngine
{
    RenderEngine_Vulkan::~RenderEngine_Vulkan()
    {
        clearVulkan();
    }

    WindowController* RenderEngine_Vulkan::createWindowController()
    {
        return registerObject(WindowControllerInfo<RenderAPI::Vulkan>::create());
    }
    RenderTarget* RenderEngine_Vulkan::createRenderTargetInternal()
    {
        return createObject<RenderTarget_Vulkan>();
    }
    RenderPipeline* RenderEngine_Vulkan::createRenderPipelineInternal()
    {
        return createObject<RenderPipeline_Vulkan>();
    }

    bool RenderEngine_Vulkan::initInternal(const jmap<window_id, WindowProperties>& windows)
    {
        if (!createVulkanInstance())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create Vulkan instance"));
            return false;
        }
        if (!Super::initInternal(windows))
        {
            clearVulkan();
            return false;
        }
        if (!pickPhysicalDevice())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to pick physical device"));
            clearVulkan();
            return false;
        }
        if (!createDevice())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan device"));
            clearVulkan();
            return false;
        }
        if (!createCommandPools())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create command pools"));
            clearVulkan();
            return false;
        }
        if (!getWindowController<WindowController_Vulkan>()->createWindowSwapchains())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan swapchains"));
            clearVulkan();
            return false;
        }
        return true;
    }

    bool RenderEngine_Vulkan::createVulkanInstance()
    {
#ifdef JDEBUG
        constexpr uint8 validationLayerCount = 1;
        const char* validationLayers[validationLayerCount] = { "VK_LAYER_KHRONOS_validation" };
        {
            uint32 layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            jarray<VkLayerProperties> availableLayers(static_cast<int32>(layerCount));
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.getData());

            bool layersFounded = true;
            for (const auto& layer : validationLayers)
            {
                bool founded = false;
                for (const auto& availableLayer : availableLayers)
                {
                    if (strcmp(layer, availableLayer.layerName) == 0)
                    {
                        founded = true;
                        break;
                    }
                }
                if (!founded)
                {
                    layersFounded = false;
                    break;
                }
            }
            if (!layersFounded)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to find validation layers"));
                return false;
            }
        }

        VkDebugUtilsMessengerCreateInfoEXT debugMessangerInfo{};
        debugMessangerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugMessangerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugMessangerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugMessangerInfo.pfnUserCallback = RenderEngine_Vulkan::Vulkan_DebugCallback;
#endif

        const jarray<const char*> extensions = getRequiredVulkanExtensions();
        VkApplicationInfo applicationInfo{};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        /* TODO: Pass application name somehow */
        applicationInfo.pApplicationName = JSTR("JumaRenderEngine");
        applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = JSTR("JumaRenderEngine");
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion = VK_API_VERSION_1_3;
        VkInstanceCreateInfo instanceInfo{};
        instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo = &applicationInfo;
        instanceInfo.enabledExtensionCount = static_cast<uint32>(extensions.getSize());
        instanceInfo.ppEnabledExtensionNames = extensions.getData();
#ifdef JDEBUG
        instanceInfo.enabledLayerCount = validationLayerCount;
        instanceInfo.ppEnabledLayerNames = validationLayers;
        instanceInfo.pNext = &debugMessangerInfo;
#else
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.ppEnabledLayerNames = nullptr;
        instanceInfo.pNext = nullptr;
#endif
        VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_VulkanInstance);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan instance"));
            return false;
        }

#ifdef JDEBUG
        result = CreateDebugUtilsMessengerEXT(m_VulkanInstance, &debugMessangerInfo, nullptr, &m_DebugMessenger);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create debug messenger"));
            return false;
        }
#endif
        return true;
    }
    jarray<const char*> RenderEngine_Vulkan::getRequiredVulkanExtensions() const
    {
        jarray<const char*> extensions = getWindowController<WindowController_Vulkan>()->getVulkanInstanceExtensions();
#ifdef JDEBUG
        extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return extensions;
    }
#ifdef JDEBUG
    VkBool32 RenderEngine_Vulkan::Vulkan_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
    {
        switch (messageSeverity)
        {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: JUMA_RENDER_LOG(warning, pCallbackData->pMessage); break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: JUMA_RENDER_LOG(error, pCallbackData->pMessage); break;
        default: ;
        }
        return VK_FALSE;
    }
#endif

    bool RenderEngine_Vulkan::pickPhysicalDevice()
    {
        uint32 deviceCount = 0;
        vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            return false;
        }

        WindowController* windowController = getWindowController(); 
        const jarray<window_id> windowIDs = windowController->getWindowIDs();
        if (windowIDs.isEmpty())
        {
            return false;
        }
        jarray<VkSurfaceKHR> windowSurfaces(windowIDs.getSize());
        for (int32 index = 0; index < windowIDs.getSize(); index++)
        {
            windowSurfaces[index] = windowController->findWindowData<WindowData_Vulkan>(windowIDs[index])->vulkanSurface;
        }

        jarray<VkPhysicalDevice> physicalDevices(static_cast<int32>(deviceCount));
        vkEnumeratePhysicalDevices(m_VulkanInstance, &deviceCount, physicalDevices.getData());
        for (const auto& physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
            if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                continue;
            }

            if constexpr (m_RequiredExtensionCount > 0)
            {
                uint32 extensionCount;
		        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		        jarray<VkExtensionProperties> availableExtensions(static_cast<int32>(extensionCount));
		        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.getData());

                bool allRequiredExtensionAailable = true;
                for (const auto& requiredExtension : m_RequiredExtensions)
                {
                    bool extensionAailable = false;
                    for (const auto& availableExtension : availableExtensions)
                    {
                        if (strcmp(requiredExtension, availableExtension.extensionName) == 0)
                        {
                            extensionAailable = true;
                            break;
                        }
                    }
                    if (!extensionAailable)
                    {
                        allRequiredExtensionAailable = false;
                        break;
                    }
                }
                if (!allRequiredExtensionAailable)
                {
                    continue;
                }
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);
            if (supportedFeatures.samplerAnisotropy != VK_TRUE)
            {
                continue;
            }

            for (const auto& surface : windowSurfaces)
            {
                uint32 surfaceFormatCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
                if (surfaceFormatCount == 0)
                {
                    continue;
                }
                uint32 surfacePresentModeCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, nullptr);
                if (surfacePresentModeCount == 0)
                {
                    continue;
                }
                if (!getQueueFamilyIndices(physicalDevice, surface, m_QueueIndices, m_Queues))
                {
                    continue;
                }

                m_PhysicalDevice = physicalDevice;
                return true;
            }
        }
        return false;
    }
    bool RenderEngine_Vulkan::getQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, 
        jmap<VulkanQueueType, int32>& outQueueIndices, jarray<VulkanQueueDescription>& outQueues)
    {
        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        jarray<VkQueueFamilyProperties> queueFamilies(static_cast<int32>(queueFamilyCount));
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.getData());

        int32 graphicsFamilyIndex = -1, transferFamilyIndex = -1;
        for (int32 queueFamilyIndex = 0; queueFamilyIndex < queueFamilies.getSize(); queueFamilyIndex++)
        {
            const VkQueueFamilyProperties& properties = queueFamilies[queueFamilyIndex];
            if ((properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)) && (properties.queueCount > 1))
            {
                graphicsFamilyIndex = transferFamilyIndex = queueFamilyIndex;
                break;
            }

            if ((graphicsFamilyIndex == -1) && (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                graphicsFamilyIndex = queueFamilyIndex;
            }
            else if ((transferFamilyIndex == -1) && (properties.queueFlags & VK_QUEUE_TRANSFER_BIT))
            {
                transferFamilyIndex = queueFamilyIndex;
            }
        }
        if ((graphicsFamilyIndex == -1) || (transferFamilyIndex == -1))
        {
            return false;
        }

        constexpr uint32 graphicsQueueIndex = 0;
        const uint32 transferQueueIndex = transferFamilyIndex != graphicsFamilyIndex ? 0 : math::min(queueFamilies[graphicsQueueIndex].queueCount - 1, graphicsQueueIndex + 1);
        if ((transferFamilyIndex != graphicsFamilyIndex) || (graphicsQueueIndex != transferQueueIndex))
        {
            outQueueIndices = {
                { VulkanQueueType::Graphics, 0 },
                { VulkanQueueType::Transfer, 1 }
            };
            outQueues = {
                { static_cast<uint32>(graphicsFamilyIndex), graphicsQueueIndex, nullptr },
                { static_cast<uint32>(transferFamilyIndex), transferQueueIndex, nullptr }
            };
        }
        else
        {
            outQueueIndices = {
                { VulkanQueueType::Graphics, 0 },
                { VulkanQueueType::Transfer, 0 }
            };
            outQueues = {
                { static_cast<uint32>(graphicsFamilyIndex), graphicsQueueIndex, nullptr }
            };
        }
        return true;
    }

    bool RenderEngine_Vulkan::createDevice()
    {
        uint32 maxQueueCount = 0;
        jmap<uint32, uint32> uniqueQueueFamilies;
        for (const auto& queue : m_Queues)
        {
            maxQueueCount = math::max(maxQueueCount, ++uniqueQueueFamilies[queue.familyIndex]);
        }

        const jarray<float> queuePriorities(maxQueueCount, 1.0f);
        jarray<VkDeviceQueueCreateInfo> queueInfos;
        queueInfos.reserve(uniqueQueueFamilies.getSize());
        for (const auto& queueFamilyIndex : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		    queueInfo.queueFamilyIndex = queueFamilyIndex.key;
		    queueInfo.queueCount = queueFamilyIndex.value;
		    queueInfo.pQueuePriorities = queuePriorities.getData();
            queueInfos.add(queueInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        VkDeviceCreateInfo deviceInfo{};
	    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	    deviceInfo.queueCreateInfoCount = static_cast<uint32>(queueInfos.getSize());
	    deviceInfo.pQueueCreateInfos = queueInfos.getData();
	    deviceInfo.pEnabledFeatures = &deviceFeatures;
	    deviceInfo.enabledExtensionCount = m_RequiredExtensionCount;
	    deviceInfo.ppEnabledExtensionNames = m_RequiredExtensions;
	    deviceInfo.enabledLayerCount = 0;
        VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan device"));
            return false;
        }

        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorInfo.instance = m_VulkanInstance;
        allocatorInfo.physicalDevice = m_PhysicalDevice;
        allocatorInfo.device = m_Device;
        result = vmaCreateAllocator(&allocatorInfo, &m_Allocator);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create VMA allocator"));
            return false;
        }
        return true;
    }

    bool RenderEngine_Vulkan::createCommandPools()
    {
        for (auto& queue : m_Queues)
        {
            vkGetDeviceQueue(m_Device, queue.familyIndex, queue.queueIndex, &queue.queue);
        }

        VulkanCommandPool* graphicsCommandPool = createObject<VulkanCommandPool>();
        if (!graphicsCommandPool->init(VulkanQueueType::Graphics))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create graphics command pool"));
            delete graphicsCommandPool;
            return false;
        }
        VulkanCommandPool* transferCommandPool = createObject<VulkanCommandPool>();
        if (!transferCommandPool->init(VulkanQueueType::Transfer, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create transfer command pool"));
            delete transferCommandPool;
            delete graphicsCommandPool;
            return false;
        }
        m_CommandPools = {
            { VulkanQueueType::Graphics, graphicsCommandPool },
            { VulkanQueueType::Transfer, transferCommandPool }
        };
        return true;
    }

    void RenderEngine_Vulkan::clearInternal()
    {
        clearVulkan();
        Super::clearInternal();
    }
    void RenderEngine_Vulkan::clearVulkan()
    {
        if (m_Device != nullptr)
        {
            vkDeviceWaitIdle(m_Device);
        }

        clearRenderAssets();

        getWindowController<WindowController_Vulkan>()->clearWindowSwapchains();

        m_RenderPasses.clear();
        m_RenderPassTypes.clear();
        m_RenderPassTypeIDs.reset();

        m_UnusedVulkanImages.clear();
        m_VulkanImages.clear();
        m_UnusedVulkanBuffers.clear();
        m_VulkanBuffers.clear();

        for (const auto& commandPool : m_CommandPools)
        {
            delete commandPool.value;
        }
        m_CommandPools.clear();
        m_Queues.clear();
        m_QueueIndices.clear();

        if (m_Device != nullptr)
        {
            if (m_Allocator != nullptr)
            {
                vmaDestroyAllocator(m_Allocator);
                m_Allocator = nullptr;
            }

            vkDestroyDevice(m_Device, nullptr);
            m_Device = nullptr;
        }
        m_PhysicalDevice = nullptr;

        clearData();

        if (m_VulkanInstance != nullptr)
        {
#ifdef JDEBUG
            if (m_DebugMessenger != nullptr)
            {
                DestroyDebugUtilsMessengerEXT(m_VulkanInstance, m_DebugMessenger, nullptr);
                m_DebugMessenger = nullptr;
            }
#endif

            vkDestroyInstance(m_VulkanInstance, nullptr);
            m_VulkanInstance = nullptr;
        }
    }

    VulkanBuffer* RenderEngine_Vulkan::getVulkanBuffer()
    {
        if (!m_UnusedVulkanBuffers.isEmpty())
        {
            VulkanBuffer* buffer = m_UnusedVulkanBuffers.getFirst();
            m_UnusedVulkanBuffers.removeFirst();
            return buffer;
        }
        return registerObject(&m_VulkanBuffers.addDefault());
    }
    VulkanImage* RenderEngine_Vulkan::getVulkanImage()
    {
        if (!m_UnusedVulkanImages.isEmpty())
        {
            VulkanImage* image = m_UnusedVulkanImages.getFirst();
            m_UnusedVulkanImages.removeFirst();
            return image;
        }
        return registerObject(&m_VulkanImages.addDefault());
    }
    void RenderEngine_Vulkan::returnVulkanBuffer(VulkanBuffer* buffer)
    {
        if (buffer == nullptr)
        {
            return;
        }
        buffer->clear();
        m_UnusedVulkanBuffers.addUnique(buffer);
    }
    void RenderEngine_Vulkan::returnVulkanImage(VulkanImage* image)
    {
        if (image == nullptr)
        {
            return;
        }
        image->clear();
        m_UnusedVulkanImages.addUnique(image);
    }

    VulkanRenderPass* RenderEngine_Vulkan::getRenderPass(const VulkanRenderPassDescription& description)
    {
        if ((description.colorFormat == VK_FORMAT_UNDEFINED) || (description.shouldUseDepth && (description.depthFormat == VK_FORMAT_UNDEFINED)))
        {
            return nullptr;
        }

        const render_pass_type_id* renderPassID = m_RenderPassTypes.find(description);
        if (renderPassID == nullptr)
        {
            renderPassID = &m_RenderPassTypes.add(description, m_RenderPassTypeIDs.getUID());
        }

        VulkanRenderPass* renderPass = m_RenderPasses.find(description);
        if (renderPass == nullptr)
        {
            renderPass = registerObject(&m_RenderPasses[description]);
            if (!renderPass->init(description, *renderPassID))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create new vulkan render pass"));
                m_RenderPasses.remove(description);
                return nullptr;
            }
        }
        return renderPass;
    }
}

#endif
