﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Material_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11.h>

#include "RenderEngine_DirectX11.h"
#include "Shader_DirectX11.h"
#include "VertexBuffer_DirectX11.h"
#include "renderEngine/Shader.h"

namespace JumaRenderEngine
{
    Material_DirectX11::~Material_DirectX11()
    {
        clearDirectX();
    }

    bool Material_DirectX11::initInternal()
    {
        ID3D11Device* device = getRenderEngine<RenderEngine_DirectX11>()->getDevice();

        const jmap<uint32, ShaderUniformBufferDescription>& uniformBufferDescriptions = getShader()->getUniformBufferDescriptions();
        if (!uniformBufferDescriptions.isEmpty())
        {
            m_UniformBuffers.reserve(uniformBufferDescriptions.getSize());
            for (const auto& uniformBufferDescription : uniformBufferDescriptions)
            {
                static constexpr uint32 mask = 15;
                const uint32 size = (uniformBufferDescription.value.size + mask) & ~mask;
                D3D11_BUFFER_DESC description{};
                description.ByteWidth = size;
                description.Usage = D3D11_USAGE_DYNAMIC;
                description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                description.MiscFlags = 0;
                description.StructureByteStride = description.ByteWidth;
                ID3D11Buffer* uniformBuffer = nullptr;
                const HRESULT result = device->CreateBuffer(&description, nullptr, &uniformBuffer);
                if (result < 0)
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 uniform buffer"));
                    continue;
                }

                m_UniformBuffers.add(uniformBufferDescription.key, { uniformBuffer, uniformBufferDescription.value.shaderStages });
            }
        }

        D3D11_DEPTH_STENCIL_DESC depthStateDescription{};
        depthStateDescription.DepthEnable = TRUE;
        depthStateDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStateDescription.DepthFunc = D3D11_COMPARISON_LESS;
        depthStateDescription.StencilEnable = FALSE;
        const HRESULT result = device->CreateDepthStencilState(&depthStateDescription, &m_DepthStencilState);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 depth stencil state"));
            clearDirectX();
            return false;
        }

        return true;
    }

    void Material_DirectX11::clearDirectX()
    {
        if (m_DepthStencilState != nullptr)
        {
            m_DepthStencilState->Release();
            m_DepthStencilState = nullptr;
        }

        for (const auto& uniformBuffer : m_UniformBuffers)
        {
            uniformBuffer.value.buffer->Release();
        }
        m_UniformBuffers.clear();
    }

    bool Material_DirectX11::bindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
        if (!getShader<Shader_DirectX11>()->bindShader(renderOptions, vertexBuffer))
        {
            return false;
        }

        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();

        updateUniformBuffersData(deviceContext);
        for (const auto& uniformBuffer : m_UniformBuffers)
        {
            if (uniformBuffer.value.shaderStages & SHADER_STAGE_VERTEX)
            {
                deviceContext->VSSetConstantBuffers(uniformBuffer.key, 1, &uniformBuffer.value.buffer);
            }
            else if (uniformBuffer.value.shaderStages & SHADER_STAGE_FRAGMENT)
            {
                deviceContext->PSSetConstantBuffers(uniformBuffer.key, 1, &uniformBuffer.value.buffer);
            }
        }

        deviceContext->OMSetDepthStencilState(m_DepthStencilState, 1);
        return true;
    }
    void Material_DirectX11::unbindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
        getShader<Shader_DirectX11>()->unbindShader(renderOptions, vertexBuffer);
    }

    void Material_DirectX11::updateUniformBuffersData(ID3D11DeviceContext* deviceContext)
    {
        jmap<uint32, D3D11_MAPPED_SUBRESOURCE> uniformBuffersData;
        const MaterialParamsStorage& materialParams = getMaterialParams();
        for (const auto& uniform : getShader()->getUniforms())
        {
            D3D11_MAPPED_SUBRESOURCE* mappedData = uniformBuffersData.find(uniform.value.shaderLocation);
            if (mappedData == nullptr)
            {
                const UniformBufferDescription* uniformBuffer = m_UniformBuffers.find(uniform.value.shaderLocation);
                if (uniformBuffer == nullptr)
                {
                    continue;
                }

                mappedData = &uniformBuffersData[uniform.value.shaderLocation];
                const HRESULT result = deviceContext->Map(uniformBuffer->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, mappedData);
                if (result < 0)
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to map DirectX11 uniform buffer data"));
                    continue;
                }
            }

            switch (uniform.value.type)
            {
            case ShaderUniformType::Float:
                {
                    ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Float>(uniform.key, value))
                    {
                        std::memcpy(mappedData + uniform.value.shaderBlockOffset, &value, sizeof(value));
                    }
                }
                break;
            case ShaderUniformType::Vec4:
                {
                    ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Vec4>(uniform.key, value))
                    {
                        std::memcpy(mappedData + uniform.value.shaderBlockOffset, &value[0], sizeof(value));
                    }
                }
                break;
            case ShaderUniformType::Mat4:
                {
                    ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Mat4>(uniform.key, value))
                    {
                        std::memcpy(mappedData + uniform.value.shaderBlockOffset, &value[0][0], sizeof(value));
                    }
                }
                break;
            default: ;
            }
        }

        for (const auto& mappedData : uniformBuffersData)
        {
            deviceContext->Unmap(m_UniformBuffers[mappedData.key].buffer, 0);
        }
    }
}

#endif
