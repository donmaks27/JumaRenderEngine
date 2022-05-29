﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "renderEngine/Shader.h"

#include <vulkan/vulkan_core.h>

namespace JumaRenderEngine
{
    class Shader_Vulkan : public Shader
    {
    public:
        Shader_Vulkan() = default;
        virtual ~Shader_Vulkan() override;
        
        VkDescriptorSetLayout getDescriptorSetLayout() const { return m_DescriptorSetLayout; }
        VkPipelineLayout getPipelineLayout() const { return m_PipelineLayout; }

        const jarray<VkPipelineShaderStageCreateInfo>& getPipelineStageInfos() const { return m_CachedPipelineStageInfos; }
        const jmap<uint32, uint32>& getUniformBufferSizes() const { return m_CachedUniformBufferSizes; }

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;

    private:

        jmap<ShaderStageFlags, VkShaderModule> m_ShaderModules;
        VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
        VkPipelineLayout m_PipelineLayout = nullptr;

        jarray<VkPipelineShaderStageCreateInfo> m_CachedPipelineStageInfos;
        jmap<uint32, uint32> m_CachedUniformBufferSizes;


        bool createShaderModules(VkDevice device, const jmap<ShaderStageFlags, jstring>& fileNames);
        bool createDescriptorSetLayout(VkDevice device);
        bool createPipelineLayout(VkDevice device);

        void clearVulkan();
    };
}

#endif
