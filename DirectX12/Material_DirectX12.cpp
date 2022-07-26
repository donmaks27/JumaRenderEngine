// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Material_DirectX12.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "RenderTarget_DirectX12.h"
#include "Shader_DirectX12.h"
#include "Texture_DirectX12.h"
#include "TextureFormat_DirectX12.h"
#include "VertexBuffer_DirectX12.h"
#include "DirectX12Objects/DirectX12PipelineStateStreamObjects.h"
#include "renderEngine/RenderTarget.h"
#include "renderEngine/Shader.h"

namespace JumaRenderEngine
{
    Material_DirectX12::~Material_DirectX12()
    {
        clearDirectX();
    }

    bool Material_DirectX12::initInternal()
    {
        const Shader_DirectX12* shader = getShader<Shader_DirectX12>();
        const jmap<jstringID, uint32>& descriptorHeapOffsets = shader->getTextureDescriptorHeapOffsets();

        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12DescriptorHeap* textureDescriptorHeap = nullptr;
        ID3D12DescriptorHeap* samplerDescriptorHeap = nullptr;
        if (!descriptorHeapOffsets.isEmpty())
        {
            textureDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorHeapOffsets.getSize(), true);
            if (textureDescriptorHeap == nullptr)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create descriptor heap for material textures"));
                return false;
            }
            samplerDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, descriptorHeapOffsets.getSize(), true);
            if (samplerDescriptorHeap == nullptr)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create descriptor heap for material samplers"));
                textureDescriptorHeap->Release();
                return false;
            }
        }

        jmap<uint32, DirectX12Buffer*> buffers;
        for (const auto& bufferDescription : shader->getUniformBufferDescriptions())
        {
            DirectX12Buffer* newBuffer = buffers.add(bufferDescription.key, renderEngine->getBuffer());
            if ((newBuffer == nullptr) || !newBuffer->initAccessedGPU(bufferDescription.value.size, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create uniform buffer"));
                for (const auto& buffer : buffers)
                {
                    renderEngine->returnBuffer(buffer.value);
                }
                if (textureDescriptorHeap != nullptr)
                {
                    textureDescriptorHeap->Release();
                    samplerDescriptorHeap->Release();
                }
                return false;
            }
        }

        m_UniformBuffers = std::move(buffers);
        m_TextureDescriptorHeap = textureDescriptorHeap;
        m_SamplerDescriptorHeap = samplerDescriptorHeap;
        return true;
    }

    void Material_DirectX12::clearDirectX()
    {
        for (const auto& pipelineState : m_PipelineStates)
        {
            pipelineState.value->Release();
        }
        m_PipelineStates.clear();

        if (!m_UniformBuffers.isEmpty())
        {
            RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
            for (const auto& buffer : m_UniformBuffers)
            {
                renderEngine->returnBuffer(buffer.value);
            }
            m_UniformBuffers.clear();
        }

        if (m_SamplerDescriptorHeap != nullptr)
        {
            m_SamplerDescriptorHeap->Release();
            m_SamplerDescriptorHeap = nullptr;
        }
        if (m_TextureDescriptorHeap != nullptr)
        {
            m_TextureDescriptorHeap->Release();
            m_TextureDescriptorHeap = nullptr;
        }
    }

    bool Material_DirectX12::bindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX12* vertexBuffer)
    {
        if (!updateUniformData())
        {
            return false;
        }

        const RenderOptions_DirectX12* renderOptionsDirectX = reinterpret_cast<const RenderOptions_DirectX12*>(renderOptions);
        const RenderTarget* renderTarget = renderOptionsDirectX->renderTarget;
        const TextureFormat colorFormat = renderTarget->getFormat();
        const TextureFormat depthFormat = TextureFormat::DEPTH24_STENCIL8;
        const TextureSamples samples = renderTarget->getSampleCount();
        ID3D12PipelineState* pipelineState = nullptr;
        if (!getPipelineState({ vertexBuffer->getVertexTypeName(), colorFormat, depthFormat, samples }, pipelineState) && (pipelineState != nullptr))
        {
            return false;
        }

        const Shader_DirectX12* shader = getShader<Shader_DirectX12>();
        ID3D12GraphicsCommandList2* commandList = renderOptionsDirectX->renderCommandList->get();
        commandList->SetGraphicsRootSignature(shader->getRootSignature());
        commandList->SetPipelineState(pipelineState);
        for (const auto& bufferParam : shader->getUniformBufferParamIndices())
        {
            commandList->SetGraphicsRootConstantBufferView(bufferParam.key, m_UniformBuffers[bufferParam.value]->get()->GetGPUVirtualAddress());
        }

        if (m_TextureDescriptorHeap != nullptr)
        {
            const uint32 paramIndex = m_UniformBuffers.getSize();

            ID3D12DescriptorHeap* const descriptorHeaps[2] = { m_TextureDescriptorHeap, m_SamplerDescriptorHeap };
            commandList->SetDescriptorHeaps(2, descriptorHeaps);
            commandList->SetGraphicsRootDescriptorTable(paramIndex, m_TextureDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
            commandList->SetGraphicsRootDescriptorTable(paramIndex + 1, m_SamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
        }
        return true;
    }

    bool Material_DirectX12::getPipelineState(const PipelineStateID& pipelineStateID, ID3D12PipelineState*& outPipelineState)
    {
        ID3D12PipelineState* const* existingPipelineState = m_PipelineStates.find(pipelineStateID);
        if (existingPipelineState != nullptr)
        {
            outPipelineState = *existingPipelineState;
            return true;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const VertexDescription* vertexDescription = renderEngine->findVertexType(pipelineStateID.vertexName);
        if (vertexDescription == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get description for vertex ") + pipelineStateID.vertexName.toString());
            return false;
        }

        const Shader_DirectX12* shader = getShader<Shader_DirectX12>();
        jarray<D3D12_INPUT_ELEMENT_DESC> inputLayouts;
        for (const auto& requiredVertexComponent : shader->getRequiredVertexComponents())
        {
            bool componentFound = false;
            for (const auto& vertexComponent : vertexDescription->components)
            {
                if (vertexComponent.name == requiredVertexComponent)
                {
                    componentFound = true;

                    DXGI_FORMAT componentFormat = DXGI_FORMAT_UNKNOWN;
                    switch (vertexComponent.type)
                    {
                    case VertexComponentType::Float: componentFormat = DXGI_FORMAT_R32_FLOAT; break;
                    case VertexComponentType::Vec2:  componentFormat = DXGI_FORMAT_R32G32_FLOAT; break;
                    case VertexComponentType::Vec3:  componentFormat = DXGI_FORMAT_R32G32B32_FLOAT; break;
                    case VertexComponentType::Vec4:  componentFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
                    default: 
                        JUMA_RENDER_LOG(error, JSTR("Unsupported type of vertex component ") + TO_JSTR(vertexComponent.shaderLocation) + JSTR(" in vertex ") + pipelineStateID.vertexName.toString());
                        return false;
                    }
                    inputLayouts.add({ 
                        "TEXCOORD", vertexComponent.shaderLocation, componentFormat, 0,
                        vertexComponent.offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
                    });
                    break;
                }
            }
            if (!componentFound)
            {
                return false;
            }
        }

        ID3D12Device2* device = renderEngine->getDevice();
        const jmap<ShaderStageFlags, ID3DBlob*>& shaderBytecodes = shader->getShaderBytecodes();
        ID3DBlob* vertexShaderBytecode = shaderBytecodes.get(SHADER_STAGE_VERTEX);
        ID3DBlob* fragmentShaderBytecode = shaderBytecodes.get(SHADER_STAGE_FRAGMENT);

        struct PipelineStateStream
        {
            DirectX12_PipelineStateStream_ROOT_SIGNATURE rootSignature;
            DirectX12_PipelineStateStream_INPUT_LAYOUT inputLayout;
            DirectX12_PipelineStateStream_PRIMITIVE_TOPOLOGY primitiveTopologyType;
            DirectX12_PipelineStateStream_RASTERIZER rasterizer;
            DirectX12_PipelineStateStream_SAMPLE_DESC sampleDescription;
            DirectX12_PipelineStateStream_VS VS;
            DirectX12_PipelineStateStream_PS PS;
            DirectX12_PipelineStateStream_DEPTH_STENCIL_FORMAT DSVFormat;
            DirectX12_PipelineStateStream_RENDER_TARGET_FORMATS RTVFormats;
        } pipelineStateStream;
        pipelineStateStream.rootSignature = shader->getRootSignature();
        pipelineStateStream.inputLayout.data.NumElements = inputLayouts.getSize();
        pipelineStateStream.inputLayout.data.pInputElementDescs = inputLayouts.getData();
        pipelineStateStream.primitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateStream.rasterizer.data.FillMode = D3D12_FILL_MODE_SOLID;
        pipelineStateStream.rasterizer.data.CullMode = D3D12_CULL_MODE_BACK;
        pipelineStateStream.rasterizer.data.FrontCounterClockwise = FALSE;
        pipelineStateStream.rasterizer.data.DepthBias = 0;
        pipelineStateStream.rasterizer.data.DepthBiasClamp = 0.0f;
        pipelineStateStream.rasterizer.data.SlopeScaledDepthBias = 0.0f;
        pipelineStateStream.rasterizer.data.DepthClipEnable = TRUE;
        pipelineStateStream.rasterizer.data.MultisampleEnable = TRUE;
        pipelineStateStream.rasterizer.data.AntialiasedLineEnable = FALSE;
        pipelineStateStream.rasterizer.data.ForcedSampleCount = 0;
        pipelineStateStream.rasterizer.data.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        pipelineStateStream.sampleDescription.data.Count = GetTextureSamplesNumber(pipelineStateID.samplesCount);
        pipelineStateStream.sampleDescription.data.Quality = 0;
        pipelineStateStream.VS.data.BytecodeLength = vertexShaderBytecode->GetBufferSize();
        pipelineStateStream.VS.data.pShaderBytecode = vertexShaderBytecode->GetBufferPointer();
        pipelineStateStream.PS.data.BytecodeLength = fragmentShaderBytecode->GetBufferSize();
        pipelineStateStream.PS.data.pShaderBytecode = fragmentShaderBytecode->GetBufferPointer();
        pipelineStateStream.DSVFormat = GetDirectX12FormatByTextureFormat(pipelineStateID.depthFormat);
        pipelineStateStream.RTVFormats.data.NumRenderTargets = 1;
        pipelineStateStream.RTVFormats.data.RTFormats[0] = GetDirectX12FormatByTextureFormat(pipelineStateID.colorFormat);
        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDescription{};
        pipelineStateStreamDescription.SizeInBytes = sizeof(pipelineStateStream);
        pipelineStateStreamDescription.pPipelineStateSubobjectStream = &pipelineStateStream;
        ID3D12PipelineState* pipelineState = nullptr;
        const HRESULT result = device->CreatePipelineState(&pipelineStateStreamDescription, IID_PPV_ARGS(&pipelineState));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 pipeline state"));
            return false;
        }

        outPipelineState = m_PipelineStates[pipelineStateID] = pipelineState;
        return true;
    }

    bool Material_DirectX12::updateUniformData()
    {
        const jset<jstringID>& notUpdatedParams = getNotUpdatedParams();
        if (notUpdatedParams.isEmpty())
        {
            return true;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();

        const Shader_DirectX12* shader = getShader<Shader_DirectX12>();
        const jmap<jstringID, uint32>& descriptorHeapOffsets = shader->getTextureDescriptorHeapOffsets();
        const MaterialParamsStorage& params = getMaterialParams();
        for (const auto& uniform : shader->getUniforms())
        {
            if (!notUpdatedParams.contains(uniform.key))
            {
                continue;
            }

            if (uniform.value.type == ShaderUniformType::Texture)
            {
                ShaderUniformInfo<ShaderUniformType::Texture>::value_type value;
                if (params.getValue<ShaderUniformType::Texture>(uniform.key, value))
                {
                    const uint32* descriptorHeapIndex = descriptorHeapOffsets.find(uniform.key);
                    if (descriptorHeapIndex == nullptr)
                    {
                        continue;
                    }

                    const Texture_DirectX12* textureValue = dynamic_cast<Texture_DirectX12*>(value);
                    if (textureValue != nullptr)
                    {
                        const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor = textureValue->getSRV()->GetCPUDescriptorHandleForHeapStart();
                        const D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor = renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(
                            m_TextureDescriptorHeap, *descriptorHeapIndex
                        );
                        device->CopyDescriptorsSimple(1, dstDescriptor, srcDescriptor, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    }
                    else
                    {
                        const RenderTarget_DirectX12* renderTargetValue = dynamic_cast<RenderTarget_DirectX12*>(value);
                        ID3D12DescriptorHeap* srv = renderTargetValue != nullptr ? renderTargetValue->getSRV() : nullptr;
                        if (srv != nullptr)
                        {
                            const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor = srv->GetCPUDescriptorHandleForHeapStart();
                            const D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor = renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(
                                m_TextureDescriptorHeap, *descriptorHeapIndex
                            );
                            device->CopyDescriptorsSimple(1, dstDescriptor, srcDescriptor, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                        }
                        else
                        {
                            // TODO: Default texture
                            continue;
                        }
                    }

                    const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptor = renderEngine->getSamplerDescription(value->getSamplerType());
                    const D3D12_CPU_DESCRIPTOR_HANDLE dstDescriptor = renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>(
                        m_SamplerDescriptorHeap, *descriptorHeapIndex
                    );
                    device->CopyDescriptorsSimple(1, dstDescriptor, srcDescriptor, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
                }
            }
            else
            {
                DirectX12Buffer* buffer = m_UniformBuffers[uniform.value.shaderLocation];
                switch (uniform.value.type)
                {
                case ShaderUniformType::Float:
                    {
                        ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                        if (params.getValue<ShaderUniformType::Float>(uniform.key, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.value.shaderBlockOffset);
                        }
                    }
                    break;
                case ShaderUniformType::Vec2:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec2>::value_type value;
                        if (params.getValue<ShaderUniformType::Vec2>(uniform.key, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.value.shaderBlockOffset);
                        }
                    }
                    break;
                case ShaderUniformType::Vec4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                        if (params.getValue<ShaderUniformType::Vec4>(uniform.key, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.value.shaderBlockOffset);
                        }
                    }
                    break;
                case ShaderUniformType::Mat4:
                    {
                        ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                        if (params.getValue<ShaderUniformType::Mat4>(uniform.key, value))
                        {
                            buffer->initMappedData();
                            buffer->setMappedData(&value, sizeof(value), uniform.value.shaderBlockOffset);
                        }
                    }
                    break;

                default: ;
                }
            }
        }

        DirectX12CommandQueue* commandQueue = renderEngine->getCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
        DirectX12CommandList* commandList = commandQueue->getCommandList();
        for (const auto& buffer : m_UniformBuffers)
        {
            buffer.value->flushMappedData(commandList, false);
        }
        commandList->execute();
        commandQueue->returnCommandList(commandList);

        clearParamsForUpdate();
        return true;
    }
}

#endif
