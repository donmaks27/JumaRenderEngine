// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/RenderEngineContextObject.h"

#include <vma/vk_mem_alloc.h>

#include "VulkanQueueType.h"

namespace JumaRenderEngine
{
    class VulkanBuffer : public RenderEngineContextObject
    {
    public:
        VulkanBuffer() = default;
        virtual ~VulkanBuffer() override;

        // Temp buffer for passing data to GPU
        bool initStaging(uint32 size);
        // Only on GPU, not updated at all
        bool initGPU(VkBufferUsageFlags usage, std::initializer_list<VulkanQueueType> accessedQueues, uint32 size, const void* data, bool waitForFinish);
        // GPU buffer, frequently writing from CPU directly. If not possible - it will be GPU with staging buffer
        bool initAccessedGPU(VkBufferUsageFlags usage, std::initializer_list<VulkanQueueType> accessedQueues, uint32 size);

        VkBuffer get() const { return m_Buffer; }

        bool initMappedData();
        bool setMappedData(const void* data, uint32 size, uint32 offset = 0);
        bool flushMappedData(bool waitForFinish);
        
        bool setData(const void* data, uint32 size, uint32 offset, bool waitForFinish);

    protected:

        virtual void clearInternal() override { clearVulkan(); }

    private:

        VkBuffer m_Buffer = nullptr;
        VmaAllocation m_Allocation = nullptr;

        uint32 m_BufferSize = 0;

        VulkanBuffer* m_StagingBuffer = nullptr;
        bool m_Mapable = false;
        void* m_MappedData = nullptr;


        void clearVulkan();

        bool setDataInternal(const void* data, uint32 size, uint32 offset);
        bool copyData(const VulkanBuffer* destinationBuffer, bool waitForFinish);
    };
}

#endif
