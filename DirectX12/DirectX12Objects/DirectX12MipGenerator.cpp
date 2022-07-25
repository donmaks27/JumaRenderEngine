// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "DirectX12MipGenerator.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <d3dcompiler.h>

#include "DirectX12MipGeneratorTarget.h"
#include "DirectX12PipelineStateStreamObjects.h"
#include "DirectX12Texture.h"
#include "renderEngine/DirectX12/RenderEngine_DirectX12.h"

namespace JumaRenderEngine
{
    enum MipSizeType
    {
        MIP_SIZE_WIDTH_HEIGHT_EVEN = 0,
        MIP_SIZE_WIDTH_ODD_HEIGHT_EVEN = 1,
        MIP_SIZE_WIDTH_EVEN_HEIGHT_ODD = 2,
        MIP_SIZE_WIDTH_HEIGHT_ODD = 3
    };
    MipSizeType GetMipSizeType(const uint32 width, const uint32 height)
    {
        if ((width % 2) == 0)
        {
            if ((height % 2) == 0)
            {
                return MIP_SIZE_WIDTH_HEIGHT_EVEN;
            }
            return MIP_SIZE_WIDTH_EVEN_HEIGHT_ODD;
        }
        if ((height % 2) == 0)
        {
            return MIP_SIZE_WIDTH_ODD_HEIGHT_EVEN;
        }
        return MIP_SIZE_WIDTH_HEIGHT_ODD;
    }
    struct alignas(uint32) MipGeneratorParams
    {
        math::vector2 texelSize = { 1.0f, 1.0f };
        uint32 mipLevelCount = 4;
        uint32 srcMipIndex = 0;
        uint32 srcMipSizeType = MIP_SIZE_WIDTH_HEIGHT_EVEN;
        uint32 isSRGB = 0;
    };

    DirectX12MipGenerator::~DirectX12MipGenerator()
    {
        clearDirectX();
    }

    bool DirectX12MipGenerator::init()
    {
        ID3DBlob* shaderBlob = nullptr;
        HRESULT result = D3DReadFileToBlob(L"shaderGenerateMips.comp.hlsl.obj", &shaderBlob);
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to read shader file"));
            return false;
        }

        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        const bool supportedRootSignature_1_1 = SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)));

        D3D12_STATIC_SAMPLER_DESC samplerDescription{};
        samplerDescription.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDescription.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDescription.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDescription.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDescription.MipLODBias = 0;
        samplerDescription.MaxAnisotropy = 1;
        samplerDescription.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        samplerDescription.MinLOD = 0.0f;
        samplerDescription.MaxLOD = D3D12_FLOAT32_MAX;
        samplerDescription.ShaderRegister = 0;
        samplerDescription.RegisterSpace = 0;
        samplerDescription.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        ID3DBlob* rootSignatureBlob = nullptr;
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription{};
        if (supportedRootSignature_1_1)
        {
            D3D12_DESCRIPTOR_RANGE1 srcDescriptorRange{};
            srcDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            srcDescriptorRange.NumDescriptors = 1;
            srcDescriptorRange.BaseShaderRegister = 0;
            srcDescriptorRange.RegisterSpace = 0;
            srcDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
            srcDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            D3D12_DESCRIPTOR_RANGE1 dstDescriptorRange{};
            dstDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            dstDescriptorRange.NumDescriptors = 4;
            dstDescriptorRange.BaseShaderRegister = 0;
            dstDescriptorRange.RegisterSpace = 0;
            dstDescriptorRange.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
            dstDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            D3D12_ROOT_PARAMETER1 params[3];
            params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            params[0].Constants.ShaderRegister = 0;
            params[0].Constants.RegisterSpace = 0;
            params[0].Constants.Num32BitValues = sizeof(MipGeneratorParams) / sizeof(uint32);
            params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[1].DescriptorTable.NumDescriptorRanges = 1;
            params[1].DescriptorTable.pDescriptorRanges = &srcDescriptorRange;
            params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[2].DescriptorTable.NumDescriptorRanges = 1;
            params[2].DescriptorTable.pDescriptorRanges = &dstDescriptorRange;
            params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            rootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            rootSignatureDescription.Desc_1_1.NumParameters = 3;
            rootSignatureDescription.Desc_1_1.pParameters = params;
            rootSignatureDescription.Desc_1_1.NumStaticSamplers = 1;
            rootSignatureDescription.Desc_1_1.pStaticSamplers = &samplerDescription;
            rootSignatureDescription.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
            result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &rootSignatureBlob, nullptr);
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to serialize root signature (1.1)"));
                shaderBlob->Release();
                return false;
            }
        }
        else
        {
            D3D12_DESCRIPTOR_RANGE srcDescriptorRange{};
            srcDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            srcDescriptorRange.NumDescriptors = 1;
            srcDescriptorRange.BaseShaderRegister = 0;
            srcDescriptorRange.RegisterSpace = 0;
            srcDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            D3D12_DESCRIPTOR_RANGE dstDescriptorRange{};
            dstDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
            dstDescriptorRange.NumDescriptors = 4;
            dstDescriptorRange.BaseShaderRegister = 0;
            dstDescriptorRange.RegisterSpace = 0;
            dstDescriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
            D3D12_ROOT_PARAMETER params[3];
            params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            params[0].Constants.ShaderRegister = 0;
            params[0].Constants.RegisterSpace = 0;
            params[0].Constants.Num32BitValues = sizeof(MipGeneratorParams) / sizeof(uint32);
            params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[1].DescriptorTable.NumDescriptorRanges = 1;
            params[1].DescriptorTable.pDescriptorRanges = &srcDescriptorRange;
            params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            params[2].DescriptorTable.NumDescriptorRanges = 1;
            params[2].DescriptorTable.pDescriptorRanges = &dstDescriptorRange;
            params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

            rootSignatureDescription.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
            rootSignatureDescription.Desc_1_0.NumParameters = 3;
            rootSignatureDescription.Desc_1_0.pParameters = params;
            rootSignatureDescription.Desc_1_0.NumStaticSamplers = 1;
            rootSignatureDescription.Desc_1_0.pStaticSamplers = &samplerDescription;
            rootSignatureDescription.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS | 
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
            result = D3D12SerializeVersionedRootSignature(&rootSignatureDescription, &rootSignatureBlob, nullptr);
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to serialize root signature (1.0)"));
                shaderBlob->Release();
                return false;
            }
        }

        ID3D12RootSignature* rootSignature = nullptr;
        result = device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(), rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
        rootSignatureBlob->Release();
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create root signature"));
            shaderBlob->Release();
            return false;
        }

        struct PipelineStateStream
        {
            DirectX12_PipelineStateStream_ROOT_SIGNATURE rootSignature;
            DirectX12_PipelineStateStream_CS CS;
        } pipelineStateStream;
        pipelineStateStream.rootSignature.data = rootSignature;
        pipelineStateStream.CS.data.BytecodeLength = shaderBlob->GetBufferSize();
        pipelineStateStream.CS.data.pShaderBytecode = shaderBlob->GetBufferPointer();
        D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDescription{};
        pipelineStateStreamDescription.SizeInBytes = sizeof(pipelineStateStream);
        pipelineStateStreamDescription.pPipelineStateSubobjectStream = &pipelineStateStream;
        ID3D12PipelineState* pipelineState = nullptr;
        result = device->CreatePipelineState(&pipelineStateStreamDescription, IID_PPV_ARGS(&pipelineState));
        shaderBlob->Release();
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create pipeline state object"));
            rootSignature->Release();
            return false;
        }

        m_ShaderRootSignature = rootSignature;
        m_ShaderPipelineState = pipelineState;
        return true;
    }

    void DirectX12MipGenerator::clearDirectX()
    {
        if (m_ShaderPipelineState != nullptr)
        {
            m_ShaderPipelineState->Release();
            m_ShaderPipelineState = nullptr;
        }
        if (m_ShaderRootSignature != nullptr)
        {
            m_ShaderRootSignature->Release();
            m_ShaderRootSignature = nullptr;
        }
    }

    void DirectX12MipGenerator::generateMips(DirectX12CommandList* commandListObject, DirectX12MipGeneratorTarget* target, 
        const D3D12_RESOURCE_STATES finalState) const
    {
        DirectX12Texture* targetTexture = target != nullptr ? target->getMipGeneratorTargetTexture() : nullptr;
        if (targetTexture == nullptr)
        {
            return;
        }
        const uint8 mipLevelsCount = targetTexture->getMipLevelsCount();
        if (mipLevelsCount <= 1)
        {
            commandListObject->changeTextureState(targetTexture, finalState);
            return;
        }

        ID3D12Resource* targetTextureResource = targetTexture->getResource();
        DirectX12Texture* stagingTexture = target->m_StagingTexture;
        ID3D12Resource* stagingTextureResource = stagingTexture->getResource();

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();
        const D3D12_RESOURCE_DESC stagingTextureResourceDesc = stagingTextureResource->GetDesc();
        jarray<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> stagingTextureFootprints(mipLevelsCount);
        device->GetCopyableFootprints(&stagingTextureResourceDesc, 0, mipLevelsCount, 0, stagingTextureFootprints.getData(), nullptr, nullptr, nullptr);

        ID3D12GraphicsCommandList2* commandList = commandListObject->get();

        commandListObject->changeTextureState(targetTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
        commandListObject->changeTextureState(stagingTexture, D3D12_RESOURCE_STATE_COPY_DEST);
        commandListObject->applyStateChanges();
        commandList->CopyResource(stagingTextureResource, targetTextureResource);

        commandList->SetComputeRootSignature(m_ShaderRootSignature);
        commandList->SetPipelineState(m_ShaderPipelineState);
        ID3D12DescriptorHeap* const heaps[] = { target->m_DescriptorHeap_SRV_UAV };
        commandList->SetDescriptorHeaps(1, heaps);

        D3D12_RESOURCE_BARRIER uavResourceBarrier{};
        uavResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        uavResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        uavResourceBarrier.UAV.pResource = stagingTextureResource;

        const uint8 descriptorSize = renderEngine->getDescriptorSize<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>();
        const D3D12_GPU_DESCRIPTOR_HANDLE srvDescriptor = target->m_DescriptorHeap_SRV_UAV->GetGPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE uavDescriptor = { srvDescriptor.ptr + descriptorSize };
        
        const uint8 stagesCount = target->m_GeneratingStagesCount;
        for (uint8 stageIndex = 0; stageIndex < stagesCount; stageIndex++)
        {
            const uint8 srcMipIndex = stageIndex * 4;
            const uint8 dstMipLevelsCount = math::min<uint8>(4, mipLevelsCount - srcMipIndex - 1);
            const D3D12_SUBRESOURCE_FOOTPRINT srcMipFootprint = stagingTextureFootprints[srcMipIndex].Footprint;
            const D3D12_SUBRESOURCE_FOOTPRINT dstMipFootprint = stagingTextureFootprints[srcMipIndex + 1].Footprint;

            MipGeneratorParams params;
            params.texelSize = { 1.0f / static_cast<float>(dstMipFootprint.Width), 1.0f / static_cast<float>(dstMipFootprint.Height) };
            params.mipLevelCount = dstMipLevelsCount;
            params.srcMipIndex = srcMipIndex;
            params.srcMipSizeType = GetMipSizeType(srcMipFootprint.Width, srcMipFootprint.Height);
            params.isSRGB = 0;
            
            commandListObject->changeTextureState(stagingTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMipIndex);
            commandListObject->changeTextureState(stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMipIndex + 1, dstMipLevelsCount);
            commandListObject->applyStateChanges();

            commandList->SetComputeRoot32BitConstants(0, sizeof(MipGeneratorParams) / sizeof(uint32), &params, 0);
            commandList->SetComputeRootDescriptorTable(1, srvDescriptor);
            commandList->SetComputeRootDescriptorTable(2, uavDescriptor);
            commandList->Dispatch((srcMipFootprint.Width + 7) / 8, (srcMipFootprint.Height + 7) / 8, 1);

            commandList->ResourceBarrier(1, &uavResourceBarrier);
            
            uavDescriptor.ptr += descriptorSize * 4;
        }

        commandListObject->changeTextureState(stagingTexture, D3D12_RESOURCE_STATE_COPY_SOURCE);
        commandListObject->changeTextureState(targetTexture, D3D12_RESOURCE_STATE_COPY_DEST);
        commandListObject->applyStateChanges();
        commandList->CopyResource(targetTextureResource, stagingTextureResource);
        commandListObject->changeTextureState(targetTexture, finalState);
    }
}

#endif
