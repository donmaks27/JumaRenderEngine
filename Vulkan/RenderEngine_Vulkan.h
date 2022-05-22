// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngine.h"

#include <vma/vk_mem_alloc.h>

#include "renderEngine/texture/TextureSamplerType.h"
#include "vulkanObjects/VulkanRenderPassDescription.h"
#include "vulkanObjects/VulkanQueueType.h"

namespace JumaRenderEngine
{
    class VulkanRenderPass;
    class VulkanCommandPool;

    struct VulkanQueueDescription
    {
        uint32 familyIndex = 0;
        uint32 queueIndex = 0;
        VkQueue queue = nullptr;
    };

    struct VertexDescription_Vulkan
    {
        VkVertexInputBindingDescription binding = VkVertexInputBindingDescription();
        jarray<VkVertexInputAttributeDescription> attributes;
    };

    class RenderEngine_Vulkan : public RenderEngine
    {
        using Super = RenderEngine;

    public:
        RenderEngine_Vulkan() = default;
        virtual ~RenderEngine_Vulkan() override;

        virtual RenderAPI getRenderAPI() const override { return RenderAPI::Vulkan; }

        VkInstance getVulkanInstance() const { return m_VulkanInstance; }
        VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
        VkDevice getDevice() const { return m_Device; }
        VmaAllocator getAllocator() const { return m_Allocator; }

        const VulkanQueueDescription* getQueue(const VulkanQueueType type) const { return !m_QueueIndices.isEmpty() ? &m_Queues[m_QueueIndices[type]] : nullptr; }
        VulkanCommandPool* getCommandPool(const VulkanQueueType type) const { return !m_CommandPools.isEmpty() ? m_CommandPools[type] : nullptr; }

        const VertexDescription_Vulkan* findVertexType_Vulkan(const jstringID& vertexName) const { return m_RegisteredVertexTypes_Vulkan.find(vertexName); }

        VulkanRenderPass* getRenderPass(const VulkanRenderPassDescription& description);

        VkSampler getTextureSampler(TextureSamplerType samplerType);

    protected:

        virtual bool initInternal(const jmap<window_id, WindowProperties>& windows) override;
        virtual void clearInternal() override;

        virtual WindowController* createWindowController() override;
        virtual VertexBuffer* createVertexBufferInternal() override;

        virtual void onRegisteredVertexType(const jstringID& vertexName) override;

    private:

        static constexpr uint8 m_RequiredExtensionCount = 1;
        static constexpr const char* m_RequiredExtensions[m_RequiredExtensionCount] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkInstance m_VulkanInstance = nullptr;
#ifdef JDEBUG
        VkDebugUtilsMessengerEXT m_DebugMessenger = nullptr;
        static VKAPI_ATTR VkBool32 VKAPI_CALL Vulkan_DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
            VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
#endif

        VkPhysicalDevice m_PhysicalDevice = nullptr;
        VkDevice m_Device = nullptr;
        VmaAllocator m_Allocator = nullptr;

        jmap<VulkanQueueType, int32> m_QueueIndices;
        jarray<VulkanQueueDescription> m_Queues;
        jmap<VulkanQueueType, VulkanCommandPool*> m_CommandPools;

        jmap<jstringID, VertexDescription_Vulkan> m_RegisteredVertexTypes_Vulkan;

        juid<render_pass_type_id> m_RenderPassTypeIDs;
        jmap<VulkanRenderPassDescription, render_pass_type_id, VulkanRenderPassDescription::compatible_predicate> m_RenderPassTypes;
        jmap<VulkanRenderPassDescription, VulkanRenderPass*, VulkanRenderPassDescription::equal_predicate> m_RenderPasses;

        jmap<TextureSamplerType, VkSampler> m_TextureSamplers;


        bool createVulkanInstance();
        jarray<const char*> getRequiredVulkanExtensions() const;
        bool initVulkanObjects();

        bool pickPhysicalDevice();
        static bool getQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, jmap<VulkanQueueType, int32>& outQueueIndices, jarray<VulkanQueueDescription>& outQueues);
        bool createDevice();
        bool createCommandPools();

        void clearVulkan();
    };
}

#endif
