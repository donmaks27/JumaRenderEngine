// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Material_Vulkan.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

#include "RenderEngine_Vulkan.h"
#include "RenderOptions_Vulkan.h"
#include "Shader_Vulkan.h"
#include "Texture_Vulkan.h"
#include "VertexBuffer_Vulkan.h"
#include "vulkanObjects/VulkanBuffer.h"
#include "vulkanObjects/VulkanCommandBuffer.h"
#include "vulkanObjects/VulkanImage.h"
#include "vulkanObjects/VulkanRenderPass.h"

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
        const Shader_Vulkan* shader = getShader<Shader_Vulkan>();
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
        const jmap<uint32, uint32>& bufferSizes = getShader<Shader_Vulkan>()->getUniformBufferSizes();
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
                    TextureBase* value = nullptr;
                    if (!params.getValue<ShaderUniformType::Texture>(uniform.key, value))
                    {
                        continue;
                    }

                    VulkanImage* vulkanImage = nullptr;
                    {
                        Texture_Vulkan* texture = dynamic_cast<Texture_Vulkan*>(value);
                        if (texture != nullptr)
                        {
                            vulkanImage = texture->getVulkanImage();
                        }
                        if (vulkanImage == nullptr)
                        {
                            continue;
                        }
                    }

                    VkDescriptorImageInfo& imageInfo = imageInfos.addDefault();
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = vulkanImage->getImageView();
                    imageInfo.sampler = renderEngine->getTextureSampler(value->getSamplerType());
                    VkWriteDescriptorSet& descriptorWrite = descriptorWrites.addDefault();
                    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    descriptorWrite.dstSet = m_DescriptorSet;
                    descriptorWrite.dstBinding = uniform.value.shaderLocation;
                    descriptorWrite.dstArrayElement = 0;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.descriptorCount = 1;
                    descriptorWrite.pImageInfo = &imageInfo;
                }
                break;
            default: ;
            }
        }

        if (!descriptorWrites.isEmpty())
        {
            vkUpdateDescriptorSets(renderEngine->getDevice(), 
               static_cast<uint32>(descriptorWrites.getSize()), descriptorWrites.getData(),
               0, nullptr
            );
        }
        if (!m_UniformBuffers.isEmpty())
        {
            for (const auto& buffer : m_UniformBuffers)
            {
                buffer.value->flushMappedData(false);
            }
            vkQueueWaitIdle(renderEngine->getQueue(VulkanQueueType::Transfer)->queue);
        }
        return true;
    }

    void Material_Vulkan::clearVulkan()
    {
        VkDevice device = getRenderEngine<RenderEngine_Vulkan>()->getDevice();

        for (const auto& renderPipeline : m_RenderPipelines)
        {
            vkDestroyPipeline(device, renderPipeline.value, nullptr);
        }
        m_RenderPipelines.clear();

        m_DescriptorSet = nullptr;

        if (m_DescriptorPool != nullptr)
        {
            vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
            m_DescriptorPool = nullptr;
        }

        for (const auto& buffer : m_UniformBuffers)
        {
            delete buffer.value;
        }
        m_UniformBuffers.clear();
    }

    bool Material_Vulkan::bindMaterial(const RenderOptions* renderOptions, VertexBuffer_Vulkan* vertexBuffer)
    {
        const RenderOptions_Vulkan* options = reinterpret_cast<const RenderOptions_Vulkan*>(renderOptions);
        const VulkanCommandBuffer* commandBuffer = options->commandBuffer;
        return bindRenderPipeline(commandBuffer->get(), vertexBuffer->getVertexTypeName(), options->renderPass) && 
            bindDescriptorSet(commandBuffer->get());
    }

    bool Material_Vulkan::bindRenderPipeline(VkCommandBuffer commandBuffer, const jstringID& vertexName, const VulkanRenderPass* renderPass)
    {
        VkPipeline pipeline;
        if (!getRenderPipeline(vertexName, renderPass, pipeline))
        {
            return false;
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        return true;
    }
    bool Material_Vulkan::getRenderPipeline(const jstringID& vertexName, const VulkanRenderPass* renderPass, VkPipeline& outPipeline)
    {
        const render_pass_type_id renderPassTypeID = renderPass->getTypeID();
        const VulkanRenderPipelineID pipelineID = { vertexName, renderPassTypeID };
        VkPipeline* pipelinePtr = m_RenderPipelines.find(pipelineID);
        if (pipelinePtr != nullptr)
        {
            outPipeline = *pipelinePtr;
            return true;
        }

        RenderEngine_Vulkan* renderEngine = getRenderEngine<RenderEngine_Vulkan>();
        const VertexDescription_Vulkan* vertexDescriptionVulkan = renderEngine->findVertexType_Vulkan(vertexName);
        const Shader_Vulkan* shader = getShader<Shader_Vulkan>();

        // Shader stages
        const jarray<VkPipelineShaderStageCreateInfo>& shaderStageInfos = shader->getPipelineStageInfos();

        // Vertex input data
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexDescriptionVulkan->binding;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32>(vertexDescriptionVulkan->attributes.getSize());
        vertexInputInfo.pVertexAttributeDescriptions = vertexDescriptionVulkan->attributes.getData();

        // Geometry type
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Depth
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.rasterizationSamples = renderPass->getDescription().sampleCount;
        multisampling.minSampleShading = 0.2f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        // Blending
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // Dynamic states
        VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32>(shaderStageInfos.getSize());
        pipelineInfo.pStages = shaderStageInfos.getData();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = shader->getPipelineLayout();
        pipelineInfo.renderPass = renderPass->get();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = nullptr;
        pipelineInfo.basePipelineIndex = -1;

        VkPipeline renderPipeline;
        const VkResult result = vkCreateGraphicsPipelines(renderEngine->getDevice(), nullptr, 1, &pipelineInfo, nullptr, &renderPipeline);
        if (result != VK_SUCCESS)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create vulkan render pipeline"));
            return false;
        }

        m_RenderPipelines[pipelineID] = renderPipeline;
        outPipeline = renderPipeline;
        return true;
    }

    bool Material_Vulkan::bindDescriptorSet(VkCommandBuffer commandBuffer)
    {
        if (!updateDescriptorSetData())
        {
            return false;
        }

        const Shader_Vulkan* shader = getShader<Shader_Vulkan>();
        vkCmdBindDescriptorSets(commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, shader->getPipelineLayout(), 
            0, 1, &m_DescriptorSet, 0, nullptr
        );
        return true;
    }
}

#endif
