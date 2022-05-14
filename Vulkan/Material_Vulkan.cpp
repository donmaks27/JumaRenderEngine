// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Material_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderEngine_Vulkan.h"
#include "Shader_Vulkan.h"
#include "vulkanObjects/VulkanBuffer.h"

namespace JumaRenderEngine
{
    Material_Vulkan::~Material_Vulkan()
    {
        clearVulkan();
    }

    bool Material_Vulkan::initInternal()
    {
        if (!Super::initInternal())
        {
            return false;
        }
        if (!createDescriptorSet())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create vulkan descriptor set"));
            clearVulkan();
            return false;
        }
        if (!initDescriptorSetData())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to init vulkan descriptor set"));
            clearVulkan();
            return false;
        }
        return true;
    }
    bool Material_Vulkan::createDescriptorSet()
    {
        const Shader_Vulkan* shader = dynamic_cast<const Shader_Vulkan*>(getShader());
        if (shader->getUniforms().isEmpty())
        {
            return true;
        }

        const uint32 bufferUniformCount = shader->getUniformBufferSizes().getSize();
        uint32 imageUniformCount = 0;
        for (const auto& uniform : shader->getUniforms())
        {
            switch (uniform.value.type)
            {
            case ShaderUniformType::Texture: 
                imageUniformCount++;
                break;
            default: ;
            }
        }

        uint8 poolSizeCount = 0;
        VkDescriptorPoolSize poolSizes[2];
        if (bufferUniformCount > 0)
        {
            poolSizeCount++;
            VkDescriptorPoolSize& poolSize = poolSizes[0];
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = bufferUniformCount;
        }
        if (imageUniformCount > 0)
        {
            poolSizeCount++;
            VkDescriptorPoolSize& poolSize = poolSizes[1];
            poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = imageUniformCount;
        }
        if (poolSizeCount == 0)
        {
            return true;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizeCount;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = 1;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        VkResult result = vkCreateDescriptorPool(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), &poolInfo, nullptr, &m_DescriptorPool);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan descriptor pool"));
            return false;
        }

        VkDescriptorSetLayout descriptorSetLayout = shader->getDescriptorSetLayout();
        VkDescriptorSetAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = m_DescriptorPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &descriptorSetLayout;
        result = vkAllocateDescriptorSets(getRenderEngine<RenderEngine_Vulkan>()->getDevice(), &allocateInfo, &m_DescriptorSet);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to allocate descriptor set"));
            return false;
        }
        return true;
    }
    bool Material_Vulkan::initDescriptorSetData()
    {
        if (m_DescriptorSet == nullptr)
        {
            return true;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const jmap<uint32, uint32>& bufferSizes = dynamic_cast<const Shader_Vulkan*>(getShader())->getUniformBufferSizes();
        if (!bufferSizes.isEmpty())
        {
            jarray<VkDescriptorBufferInfo> bufferInfos;
            jarray<VkWriteDescriptorSet> descriptorWrites;
            m_UniformBuffers.reserve(bufferSizes.getSize());
            bufferInfos.reserve(bufferSizes.getSize());
            descriptorWrites.reserve(bufferSizes.getSize());
            for (const auto& bufferSize : bufferSizes)
            {
                VulkanBuffer* buffer = renderEngine->createObject<VulkanBuffer>();
                if (!buffer->initAccessedGPU(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, { VulkanQueueType::Graphics }, bufferSize.value))
                {
                    JUMA_RENDER_LOG(error, JSTR("Failed to initialize one of the vulkan uniform buffers"));
                    delete buffer;
                    return false;
                }
                m_UniformBuffers.add(bufferSize.key, buffer);

                VkDescriptorBufferInfo& bufferInfo = bufferInfos.addDefault();
                bufferInfo.buffer = buffer->get();
                bufferInfo.offset = 0;
                bufferInfo.range = VK_WHOLE_SIZE;
                VkWriteDescriptorSet& descriptorWrite = descriptorWrites.addDefault();
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo = &bufferInfo;
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_DescriptorSet;
                descriptorWrite.dstBinding = bufferSize.key;
                descriptorWrite.dstArrayElement = 0;
            }
            if (!descriptorWrites.isEmpty())
            {
                vkUpdateDescriptorSets(renderEngine->getDevice(), 
                   static_cast<uint32>(descriptorWrites.getSize()), descriptorWrites.getData(),
                   0, nullptr
                );
            }
        }
        return updateDescriptorSetData();
    }
    bool Material_Vulkan::updateDescriptorSetData()
    {
        if (m_DescriptorSet == nullptr)
        {
            return true;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const jmap<jstringID, ShaderUniform>& uniforms = getShader()->getUniforms();
        const MaterialParamsStorage& params = getMaterialParams();

        jarray<VkDescriptorImageInfo> imageInfos;
        jarray<VkWriteDescriptorSet> descriptorWrites;
        imageInfos.reserve(uniforms.getSize());
        descriptorWrites.reserve(uniforms.getSize());
        for (const auto& uniform : uniforms)
        {
            switch (uniform.value.type)
            {
            case ShaderUniformType::Float:
                {
                    float value;
                    if (!params.getValue<ShaderUniformType::Float>(uniform.key, value))
                    {
                        continue;
                    }

                    VulkanBuffer* buffer = m_UniformBuffers[uniform.value.shaderLocation];
                    buffer->initMappedData();
                    buffer->setMappedData(&value, sizeof(value), uniform.value.shaderBlockOffset);
                }
                break;
            case ShaderUniformType::Vec4:
                {
                    math::vector4 value;
                    if (!params.getValue<ShaderUniformType::Vec4>(uniform.key, value))
                    {
                        continue;
                    }

                    VulkanBuffer* buffer = m_UniformBuffers[uniform.value.shaderLocation];
                    buffer->initMappedData();
                    buffer->setMappedData(&value, sizeof(value), uniform.value.shaderBlockOffset);
                }
                break;
            case ShaderUniformType::Mat4:
                {
                    math::matrix4 value;
                    if (!params.getValue<ShaderUniformType::Mat4>(uniform.key, value))
                    {
                        continue;
                    }

                    VulkanBuffer* buffer = m_UniformBuffers[uniform.value.shaderLocation];
                    buffer->initMappedData();
                    buffer->setMappedData(&value, sizeof(value), uniform.value.shaderBlockOffset);
                }
                break;

            case ShaderUniformType::Texture:
                {
                    // TODO: Load textures
                }
                break;
            default: ;
            }
        }

        if (!m_UniformBuffers.isEmpty())
        {
            for (const auto& buffer : m_UniformBuffers)
            {
                buffer.value->flushMappedData(false);
            }
            vkQueueWaitIdle(renderEngine->getQueue(VulkanQueueType::Transfer)->queue);
        }

        if (!descriptorWrites.isEmpty())
        {
            vkUpdateDescriptorSets(renderEngine->getDevice(), 
               static_cast<uint32>(descriptorWrites.getSize()), descriptorWrites.getData(),
               0, nullptr
            );
        }
        return true;
    }

    void Material_Vulkan::clearVulkan()
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        if (m_DescriptorPool != nullptr)
        {
            vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
        }
    }
}

#endif
