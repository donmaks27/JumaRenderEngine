// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/Shader.h"

#include <d3dcompiler.h>

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX11;
    struct RenderOptions;

    class Shader_DirectX11 : public Shader
    {
    public:
        Shader_DirectX11() = default;
        virtual ~Shader_DirectX11() override;

        bool bindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);
        void unbindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);

    protected:

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) override;

    private:

        ID3DBlob* m_VertexShaderBlob = nullptr;
        ID3D11VertexShader* m_VertexShader = nullptr;
        ID3D11PixelShader* m_FragmentShader = nullptr;

        jmap<jstringID, ID3D11InputLayout*> m_VertexInputLayouts;


        void clearDirectX();
        
        ID3D11InputLayout* getVertexInputLayout(const jstringID& vertexName);
    };
}

#endif
