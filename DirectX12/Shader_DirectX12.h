// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/Shader.h"

#include <d3dcommon.h>

struct ID3D12RootSignature;

namespace JumaRenderEngine
{
    class Shader_DirectX12 : public Shader
    {
    public:
        Shader_DirectX12() = default;
        virtual ~Shader_DirectX12() override;

        ID3D12RootSignature* getRootSignature() const { return m_RootSignature; }

        const jmap<ShaderStageFlags, ID3DBlob*>& getShaderBytecodes() const { return m_ShaderBytecodes; }
        const jmap<uint32, uint32>& getUniformBufferParamIndices() const { return m_UniformBufferParamIndices; }
        const jmap<jstringID, uint32>& getTextureDescriptorHeapOffsets() const { return m_TextureDescriptorHeapOffsets; }

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;

    private:

        ID3D12RootSignature* m_RootSignature = nullptr;

        jmap<ShaderStageFlags, ID3DBlob*> m_ShaderBytecodes;
        jmap<uint32, uint32> m_UniformBufferParamIndices;
        jmap<jstringID, uint32> m_TextureDescriptorHeapOffsets;


        void clearDirectX();
    };
}

#endif
