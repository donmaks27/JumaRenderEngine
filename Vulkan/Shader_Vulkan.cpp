// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Shader_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include <fstream>

#include "RenderEngine_Vulkan.h"
#include "renderEngine/material/ShaderUniformInfo.h"

namespace JumaRenderEngine
{
    bool CreateVulkanShaderModule(VkShaderModule& outShaderModule, VkDevice device, const jstring& fileName, const bool optional)
    {
        std::ifstream file(*fileName, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            if (!optional)
            {
                JUMA_RENDER_LOG(error, JSTR("Can't open file ") + fileName);
                return false;
            }
            outShaderModule = nullptr;
            return true;
        }

        jarray<char> data(static_cast<int32>(file.tellg()), 0);
        if (!data.isEmpty())
        {
            file.seekg(0, std::ios::beg);
            file.read(data.getData(), data.getSize());
        }
        file.close();
        if (data.isEmpty())
        {
            JUMA_RENDER_LOG(error, JSTR("Empty shader file ") + fileName);
            return false;
        }

        VkShaderModuleCreateInfo shaderInfo{};
        shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderInfo.codeSize = data.getSize();
        shaderInfo.pCode = reinterpret_cast<const uint32*>(data.getData());
        const VkResult result = vkCreateShaderModule(device, &shaderInfo, nullptr, &outShaderModule);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create shader module ") + fileName);
            return false;
        }
        return true;
    }
    bool CreateVulkanShaderModule(VkShaderModule& outShaderModule, VkDevice device, const jmap<ShaderStageFlags, jstring>& fileNames, 
        const ShaderStageFlags shaderStage, const jstring& fileNamePostfix, const bool optional)
    {
        const jstring* fileName = fileNames.find(shaderStage);
        if (fileName == nullptr)
        {
            if (!optional)
            {
                JUMA_RENDER_LOG(error, JSTR("Missed file for required shader stage"));
                return false;
            }
            outShaderModule = nullptr;
            return true;
        }
        return CreateVulkanShaderModule(outShaderModule, device, *fileName + fileNamePostfix, optional);
    }

    Shader_Vulkan::~Shader_Vulkan()
    {
        clearVulkan();
    }

    bool Shader_Vulkan::initInternal(const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();
        if (!createShaderModules(device, fileNames))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan shader modules"));
            return false;
        }
        if (!createDescriptorSetLayout(device))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan descriptor set layout"));
            clearVulkan();
            return false;
        }
        if (!createPipelineLayout(device))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan pipeline layout"));
            clearVulkan();
            return false;
        }
        return true;
    }
    bool Shader_Vulkan::createShaderModules(VkDevice device, const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        VkShaderModule modules[2] = { nullptr, nullptr };
        if (!CreateVulkanShaderModule(modules[0], device, fileNames, SHADER_STAGE_VERTEX, ".vert.spv", false))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan vertex shader module"));
            return false;
        }
        if (!CreateVulkanShaderModule(modules[1], device, fileNames, SHADER_STAGE_FRAGMENT, ".frag.spv", false))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan fragment shader module"));
            vkDestroyShaderModule(device, modules[0], nullptr);
            return false;
        }
        m_ShaderModules = { { SHADER_STAGE_VERTEX, modules[0] }, { SHADER_STAGE_FRAGMENT, modules[1] } };

        m_CachedPipelineStageInfos.reserve(m_ShaderModules.getSize());
        for (const auto& shaderModule : m_ShaderModules)
        {
            VkPipelineShaderStageCreateInfo& stageInfo = m_CachedPipelineStageInfos.addDefault();
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.module = shaderModule.value;
            stageInfo.pName = "main";
            switch (shaderModule.key)
            {
            case SHADER_STAGE_VERTEX: stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
            case SHADER_STAGE_FRAGMENT: stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
            default: ;
            }
            stageInfo.flags = 0;
        }
        return true;
    }
    bool Shader_Vulkan::createDescriptorSetLayout(VkDevice device)
    {
        const jmap<jstringID, ShaderUniform>& uniforms = getUniforms();
        if (uniforms.isEmpty())
        {
            return true;
        }

        jarray<VkDescriptorSetLayoutBinding> layoutBindings;
        jmap<uint32, int32> layoutBindingsMap;
        for (const auto& uniform : uniforms)
        {
            int32& index = layoutBindingsMap[uniform.value.shaderLocation];
            if (!layoutBindings.isValidIndex(index))
            {
                index = layoutBindings.getSize();
                layoutBindings.addDefault().stageFlags = 0;
            }

            VkDescriptorSetLayoutBinding& layoutBinding = layoutBindings[index];
            layoutBinding.binding = uniform.value.shaderLocation;
            layoutBinding.pImmutableSamplers = nullptr;
            if (uniform.value.shaderStages & SHADER_STAGE_VERTEX)
            {
                layoutBinding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
            }
            if (uniform.value.shaderStages & SHADER_STAGE_FRAGMENT)
            {
                layoutBinding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
            }
            layoutBinding.descriptorType = IsShaderUniformScalar(uniform.value.type) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            layoutBinding.descriptorCount = 1;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32>(layoutBindings.getSize());
        layoutInfo.pBindings = layoutBindings.getData();
        const VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_DescriptorSetLayout);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan descriptor set layout"));
            return false;
        }
        return true;
    }
    bool Shader_Vulkan::createPipelineLayout(VkDevice device)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if (m_DescriptorSetLayout != nullptr)
        {
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
        }
        else
        {
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = nullptr;
        }
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        const VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan pipeline layout"));
            return false;
        }
        return true;
    }

    void Shader_Vulkan::clearVulkan()
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        m_CachedPipelineStageInfos.clear();

        if (m_PipelineLayout != nullptr)
        {
            vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
            m_PipelineLayout = nullptr;
        }
        if (m_DescriptorSetLayout != nullptr)
        {
            vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
            m_DescriptorSetLayout = nullptr;
        }
        for (const auto& shaderModule : m_ShaderModules)
        {
            if (shaderModule.value != nullptr)
            {
                vkDestroyShaderModule(device, shaderModule.value, nullptr);
            }
        }
        m_ShaderModules.clear();
    }
}

#endif
