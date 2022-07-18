// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Shader_DirectX12.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <d3dcompiler.h>

#include "RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    bool CheckedRootSignatureSupport = false;
    bool SupportedRootSignature_1_1 = false;

    ID3DBlob* LoadDirectX12ShaderFile(const jstring& fileName, const bool optional)
    {
        std::wstring fileNameWide;
        fileNameWide.resize(fileName.getSize() + 1);
        const int32 fileNameWideSize = MultiByteToWideChar(CP_UTF8, 0, *fileName, fileName.getSize(), &fileNameWide[0], static_cast<int>(fileNameWide.size()));
        if (fileNameWideSize <= 0)
        {
            if (!optional)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to convert UTF-8 file name ") + fileName + JSTR(" to WCHAR"));
            }
            return nullptr;
        }

        ID3DBlob* shaderBlob = nullptr;
        const HRESULT result = D3DReadFileToBlob(fileNameWide.c_str(), &shaderBlob);
        if (result < 0)
        {
            if (!optional)
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed read shader file ") + fileName + JSTR(" into DirectX11 blob"));
            }
            return nullptr;
        }

        return shaderBlob;
    }
    ID3DBlob* LoadDirectX12ShaderFile(const jmap<ShaderStageFlags, jstring>& fileNames, const ShaderStageFlags shaderStage, 
        const jstring& filePostfix, const bool optional)
    {
        const jstring* fileName = fileNames.find(shaderStage);
        if (fileName == nullptr)
        {
            if (!optional)
            {
                JUMA_RENDER_LOG(error, JSTR("Missed file name for required shader stage"));
            }
            return nullptr;
        }
        return LoadDirectX12ShaderFile(*fileName + filePostfix, optional);
    }

    constexpr D3D12_SHADER_VISIBILITY GetDirectX12ShaderParamVisibility(const uint8 stages)
    {
        const bool vertexShaderVisible = (stages & SHADER_STAGE_VERTEX) != 0;
        const bool fragmentShaderVisible = (stages & SHADER_STAGE_FRAGMENT) != 0;
        if (vertexShaderVisible && fragmentShaderVisible)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        return fragmentShaderVisible ? D3D12_SHADER_VISIBILITY_PIXEL : D3D12_SHADER_VISIBILITY_VERTEX;
    }
    ID3DBlob* CreateDirectX12RootSignatureBlob(const jmap<uint32, ShaderUniformBufferDescription>& uniformBuffers, 
        const jmap<jstringID, ShaderUniform>& uniforms, jmap<jstringID, uint32>& outDescriptorHeapOffsets)
    {
        ID3DBlob* resultBlob = nullptr;
        HRESULT result = 0;
        if (SupportedRootSignature_1_1)
        {
            jarray<D3D12_ROOT_PARAMETER1> rootSignatureParams;
            for (const auto& bufferDescription : uniformBuffers)
            {
                D3D12_ROOT_PARAMETER1& parameter = rootSignatureParams.addDefault();
                parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                parameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(bufferDescription.value.shaderStages);
                parameter.Descriptor.ShaderRegister = bufferDescription.key;
                parameter.Descriptor.RegisterSpace = 0;
                parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
            }

            jarray<D3D12_DESCRIPTOR_RANGE1> textureDescriptorRanges;
            jarray<D3D12_DESCRIPTOR_RANGE1> samplerDescriptorRanges;
            uint8 shaderStages = 0;
            for (const auto& uniform : uniforms)
            {
                if (uniform.value.type != ShaderUniformType::Texture)
                {
                    continue;
                }
                shaderStages |= uniform.value.shaderStages;

                const uint32 offset = outDescriptorHeapOffsets.add(uniform.key, textureDescriptorRanges.getSize());
                D3D12_DESCRIPTOR_RANGE1& textureDescriptorRange = textureDescriptorRanges.addDefault();
                textureDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                textureDescriptorRange.NumDescriptors = 1;
                textureDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                textureDescriptorRange.RegisterSpace = 0;
                textureDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                textureDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
                D3D12_DESCRIPTOR_RANGE1& samplerDescriptorRange = samplerDescriptorRanges.addDefault();
                samplerDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                samplerDescriptorRange.NumDescriptors = 1;
                samplerDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                samplerDescriptorRange.RegisterSpace = 0;
                samplerDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
                samplerDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
            }
            if (!textureDescriptorRanges.isEmpty() && (shaderStages != 0))
            {
                D3D12_ROOT_PARAMETER1& textureParameter = rootSignatureParams.addDefault();
                textureParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                textureParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                textureParameter.DescriptorTable.NumDescriptorRanges = textureDescriptorRanges.getSize();
                textureParameter.DescriptorTable.pDescriptorRanges = textureDescriptorRanges.getData();

                D3D12_ROOT_PARAMETER1& samplerParameter = rootSignatureParams.addDefault();
                samplerParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                samplerParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                samplerParameter.DescriptorTable.NumDescriptorRanges = samplerDescriptorRanges.getSize();
                samplerParameter.DescriptorTable.pDescriptorRanges = samplerDescriptorRanges.getData();
            }

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription{};
            rootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            rootSignatureDescription.Desc_1_1.NumParameters = rootSignatureParams.getSize();
            rootSignatureDescription.Desc_1_1.pParameters = rootSignatureParams.getData();
            rootSignatureDescription.Desc_1_1.NumStaticSamplers = 0;
            rootSignatureDescription.Desc_1_1.pStaticSamplers = nullptr;
            rootSignatureDescription.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &resultBlob, nullptr);
        }
        else
        {
            jarray<D3D12_ROOT_PARAMETER> rootSignatureParams;
            for (const auto& bufferDescription : uniformBuffers)
            {
                D3D12_ROOT_PARAMETER& parameter = rootSignatureParams.addDefault();
                parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
                parameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(bufferDescription.value.shaderStages);
                parameter.Descriptor.ShaderRegister = bufferDescription.key;
                parameter.Descriptor.RegisterSpace = 0;
            }

            jarray<D3D12_DESCRIPTOR_RANGE> textureDescriptorRanges;
            jarray<D3D12_DESCRIPTOR_RANGE> samplerDescriptorRanges;
            uint8 shaderStages = 0;
            for (const auto& uniform : uniforms)
            {
                if (uniform.value.type != ShaderUniformType::Texture)
                {
                    continue;
                }
                shaderStages |= uniform.value.shaderStages;

                const uint32 offset = outDescriptorHeapOffsets.add(uniform.key, textureDescriptorRanges.getSize());
                D3D12_DESCRIPTOR_RANGE& textureDescriptorRange = textureDescriptorRanges.addDefault();
                textureDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                textureDescriptorRange.NumDescriptors = 1;
                textureDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                textureDescriptorRange.RegisterSpace = 0;
                textureDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
                D3D12_DESCRIPTOR_RANGE& samplerDescriptorRange = samplerDescriptorRanges.addDefault();
                samplerDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
                samplerDescriptorRange.NumDescriptors = 1;
                samplerDescriptorRange.BaseShaderRegister = uniform.value.shaderLocation;
                samplerDescriptorRange.RegisterSpace = 0;
                samplerDescriptorRange.OffsetInDescriptorsFromTableStart = offset;
            }
            if (!textureDescriptorRanges.isEmpty() && (shaderStages != 0))
            {
                D3D12_ROOT_PARAMETER& textureParameter = rootSignatureParams.addDefault();
                textureParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                textureParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                textureParameter.DescriptorTable.NumDescriptorRanges = textureDescriptorRanges.getSize();
                textureParameter.DescriptorTable.pDescriptorRanges = textureDescriptorRanges.getData();

                D3D12_ROOT_PARAMETER& samplerParameter = rootSignatureParams.addDefault();
                samplerParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                samplerParameter.ShaderVisibility = GetDirectX12ShaderParamVisibility(shaderStages);
                samplerParameter.DescriptorTable.NumDescriptorRanges = samplerDescriptorRanges.getSize();
                samplerParameter.DescriptorTable.pDescriptorRanges = samplerDescriptorRanges.getData();
            }

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription{};
            rootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
            rootSignatureDescription.Desc_1_0.NumParameters = rootSignatureParams.getSize();
            rootSignatureDescription.Desc_1_0.pParameters = rootSignatureParams.getData();
            rootSignatureDescription.Desc_1_0.NumStaticSamplers = 0;
            rootSignatureDescription.Desc_1_0.pStaticSamplers = nullptr;
            rootSignatureDescription.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &resultBlob, nullptr);
        }
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to serialize root signature"));
            return nullptr;
        }
        return resultBlob;
    }

    Shader_DirectX12::~Shader_DirectX12()
    {
        clearDirectX();
    }

    bool Shader_DirectX12::initInternal(const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        ID3DBlob* vertexShaderBlob = LoadDirectX12ShaderFile(fileNames, SHADER_STAGE_VERTEX, JSTR(".vert.hlsl.obj"), false);
        if (vertexShaderBlob == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to load DirectX12 vertex shader"));
            return false;
        }
        ID3DBlob* fragmentShaderBlob = LoadDirectX12ShaderFile(fileNames, SHADER_STAGE_FRAGMENT, JSTR(".frag.hlsl.obj"), false);
        if (fragmentShaderBlob == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to load DirectX12 fragment shader"));
            vertexShaderBlob->Release();
            return false;
        }
        
        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();
        if (!CheckedRootSignatureSupport)
        {
            D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
            SupportedRootSignature_1_1 = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));
            CheckedRootSignatureSupport = true;
        }

        jmap<jstringID, uint32> descriptorHeapOffsets;
        ID3DBlob* rootSignatureBlob = CreateDirectX12RootSignatureBlob(getUniformBufferDescriptions(), getUniforms(), descriptorHeapOffsets);
        if (rootSignatureBlob == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 root signature blob"));
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        ID3D12RootSignature* rootSignature = nullptr;
        const HRESULT result = device->CreateRootSignature(
            0, rootSignatureBlob->GetBufferPointer(), 
            rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature)
        );
        rootSignatureBlob->Release();
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 root signature"));
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        m_RootSignature = rootSignature;
        m_ShaderBytecodes = { { SHADER_STAGE_VERTEX, vertexShaderBlob }, { SHADER_STAGE_FRAGMENT, fragmentShaderBlob } };
        m_TextureDescriptorHeapOffsets = std::move(descriptorHeapOffsets);
        return true;
    }

    void Shader_DirectX12::clearDirectX()
    {
        m_TextureDescriptorHeapOffsets.clear();
        for (const auto& bytecode : m_ShaderBytecodes)
        {
            bytecode.value->Release();
        }
        m_ShaderBytecodes.clear();
        if (m_RootSignature != nullptr)
        {
            m_RootSignature->Release();
            m_RootSignature = nullptr;
        }
    }
}

#endif
