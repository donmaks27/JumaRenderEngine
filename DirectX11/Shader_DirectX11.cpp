// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Shader_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11.h>

#include "RenderEngine_DirectX11.h"
#include "VertexBuffer_DirectX11.h"

namespace JumaRenderEngine
{
    ID3DBlob* LoadDirectX11ShaderFile(const jstring& fileName, const bool optional)
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
    ID3DBlob* LoadDirectX11ShaderFile(const jmap<ShaderStageFlags, jstring>& fileNames, const ShaderStageFlags shaderStage, 
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
        return LoadDirectX11ShaderFile(*fileName + filePostfix, optional);
    }

    Shader_DirectX11::~Shader_DirectX11()
    {
        clearDirectX();
    }

    bool Shader_DirectX11::initInternal(const jmap<ShaderStageFlags, jstring>& fileNames)
    {
        ID3DBlob* vertexShaderBlob = LoadDirectX11ShaderFile(fileNames, SHADER_STAGE_VERTEX, JSTR(".vert.hlsl.obj"), false);
        if (vertexShaderBlob == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to load DirectX11 vertex shader"));
            return false;
        }
        ID3DBlob* fragmentShaderBlob = LoadDirectX11ShaderFile(fileNames, SHADER_STAGE_FRAGMENT, JSTR(".frag.hlsl.obj"), false);
        if (fragmentShaderBlob == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to load DirectX11 fragment shader"));
            vertexShaderBlob->Release();
            return false;
        }

        const RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        ID3D11Device* device = renderEngine->getDevice();

        ID3D11VertexShader* vertexShader = nullptr;
        HRESULT result = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &vertexShader);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 vertex shader"));
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        ID3D11PixelShader* fragmentShader = nullptr;
        result = device->CreatePixelShader(fragmentShaderBlob->GetBufferPointer(), fragmentShaderBlob->GetBufferSize(), nullptr, &fragmentShader);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 vertex shader"));
            vertexShader->Release();
            fragmentShaderBlob->Release();
            vertexShaderBlob->Release();
            return false;
        }

        fragmentShaderBlob->Release();

        m_VertexShaderBlob = vertexShaderBlob;
        m_VertexShader = vertexShader;
        m_FragmentShader = fragmentShader;
        return true;
    }

    void Shader_DirectX11::clearDirectX()
    {
        for (const auto& inputLayout : m_VertexInputLayouts)
        {
            inputLayout.value->Release();
        }
        m_VertexInputLayouts.clear();

        if (m_FragmentShader != nullptr)
        {
            m_FragmentShader->Release();
            m_FragmentShader = nullptr;
        }
        if (m_VertexShader != nullptr)
        {
            m_VertexShader->Release();
            m_VertexShader = nullptr;
        }
        if (m_VertexShaderBlob != nullptr)
        {
            m_VertexShaderBlob->Release();
            m_VertexShaderBlob = nullptr;
        }
    }

    ID3D11InputLayout* Shader_DirectX11::getVertexInputLayout(const jstringID& vertexName)
    {
        ID3D11InputLayout** existingInputLayoutPtr = m_VertexInputLayouts.find(vertexName);
        if (existingInputLayoutPtr != nullptr)
        {
            return *existingInputLayoutPtr;
        }

        if (m_VertexShaderBlob == nullptr)
        {
            return nullptr;
        }
        const RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        const VertexDescription* vertexDescription = renderEngine->findVertexType(vertexName);
        if (vertexDescription == nullptr)
        {
            return nullptr;
        }

        jarray<jstring> semanticNames;
        jarray<D3D11_INPUT_ELEMENT_DESC> vertexLayoutDescriptions;
        vertexLayoutDescriptions.reserve(vertexDescription->components.getSize());
        for (const auto& vertexComponent : vertexDescription->components)
        {
            DXGI_FORMAT componentFormat;
            switch (vertexComponent.type)
            {
            case VertexComponentType::Float: componentFormat = DXGI_FORMAT_R32_FLOAT; break;
            case VertexComponentType::Vec2: componentFormat = DXGI_FORMAT_R32G32_FLOAT; break;
            case VertexComponentType::Vec3: componentFormat = DXGI_FORMAT_R32G32B32_FLOAT; break;
            case VertexComponentType::Vec4: componentFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
            default: continue;
            }

            vertexLayoutDescriptions.add({
                JSTR("TEXCOORD"), vertexComponent.shaderLocation, componentFormat, 0, vertexComponent.offset, D3D11_INPUT_PER_VERTEX_DATA, 0
            });
        }

        ID3D11InputLayout* inputLayout = nullptr;
        const HRESULT result = renderEngine->getDevice()->CreateInputLayout(
            vertexLayoutDescriptions.getData(), vertexLayoutDescriptions.getSize(), 
            m_VertexShaderBlob->GetBufferPointer(), m_VertexShaderBlob->GetBufferSize(), 
            &inputLayout
        );
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 input layout for vertex ") + vertexName.toString());
            return nullptr;
        }
        return m_VertexInputLayouts[vertexName] = inputLayout;
    }

    bool Shader_DirectX11::bindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
        ID3D11InputLayout* inputLayout = getVertexInputLayout(vertexBuffer->getVertexTypeName());
        if (inputLayout == nullptr)
        {
            return false;
        }

        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();
        deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
        deviceContext->PSSetShader(m_FragmentShader, nullptr, 0);
        deviceContext->IASetInputLayout(inputLayout);
        return true;
    }
    void Shader_DirectX11::unbindShader(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer)
    {
    }
}

#endif
