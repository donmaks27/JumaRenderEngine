// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderEngine_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11.h>

#include "Material_DirectX11.h"
#include "RenderTarget_DirectX11.h"
#include "Shader_DirectX11.h"
#include "Texture_DirectX11.h"
#include "VertexBuffer_DirectX11.h"
#include "renderEngine/window/DirectX11/WindowControllerInfo_DirectX11.h"

namespace JumaRenderEngine
{
    RenderEngine_DirectX11::~RenderEngine_DirectX11()
    {
        clearDirectX();
    }

    bool RenderEngine_DirectX11::initInternal(const jmap<window_id, WindowProperties>& windows)
    {
        if (!Super::initInternal(windows))
        {
            return false;
        }
        if (!createDirectXDevice())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create device"));
            return false;
        }
        if (!getWindowController<WindowController_DirectX11>()->createWindowSwapchains())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX11 swapchains"));
            return false;
        }
        return true;
    }
    bool RenderEngine_DirectX11::createDirectXDevice()
    {
#ifdef JDEBUG
        constexpr UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
        constexpr UINT createDeviceFlags = 0;
#endif
        constexpr D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        const HRESULT result = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 
            createDeviceFlags, featureLevels, 4, D3D11_SDK_VERSION, 
            &m_Device, nullptr, &m_DeviceContext
        );
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 device"));
            return false;
        }
        return true;
    }

    void RenderEngine_DirectX11::clearInternal()
    {
        clearDirectX();
        Super::clearInternal();
    }
    void RenderEngine_DirectX11::clearDirectX()
    {
        clearRenderAssets();
        {
            WindowController_DirectX11* windowController = getWindowController<WindowController_DirectX11>();
            if (windowController != nullptr)
            {
                windowController->clearWindowSwapchains();
            }
        }

        for (const auto& textureSampler : m_TextureSamplers)
        {
            textureSampler.value->Release();
        }
        m_TextureSamplers.clear();
        
        if (m_DeviceContext != nullptr)
        {
            m_DeviceContext->Release();
            m_DeviceContext = nullptr;
        }
        if (m_Device != nullptr)
        {
            m_Device->Release();
            m_Device = nullptr;
        }
    }

    WindowController* RenderEngine_DirectX11::createWindowController()
    {
        return registerObject(WindowControllerInfo<RenderAPI::DirectX11>::create());
    }
    VertexBuffer* RenderEngine_DirectX11::createVertexBufferInternal()
    {
        return createObject<VertexBuffer_DirectX11>();
    }
    Texture* RenderEngine_DirectX11::createTextureInternal()
    {
        return createObject<Texture_DirectX11>();
    }
    Shader* RenderEngine_DirectX11::createShaderInternal()
    {
        return createObject<Shader_DirectX11>();
    }
    Material* RenderEngine_DirectX11::createMaterialInternal()
    {
        return createObject<Material_DirectX11>();
    }
    RenderTarget* RenderEngine_DirectX11::createRenderTargetInternal()
    {
        return createObject<RenderTarget_DirectX11>();
    }

    ID3D11SamplerState* RenderEngine_DirectX11::getTextureSampler(const TextureSamplerType samplerType)
    {
        ID3D11SamplerState** samplerPtr = m_TextureSamplers.find(samplerType);
        if (samplerPtr != nullptr)
        {
            return *samplerPtr;
        }

        D3D11_SAMPLER_DESC samplerDescription{};
        switch (samplerType.filterType)
        {
        case TextureFilterType::Point:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            samplerDescription.MaxAnisotropy = 1;
            break;
        case TextureFilterType::Bilinear:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            samplerDescription.MaxAnisotropy = 1;
            break;
        case TextureFilterType::Trilinear:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 1;
            break;
        case TextureFilterType::Anisotropic_2:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 2;
            break;
        case TextureFilterType::Anisotropic_4:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 4;
            break;
        case TextureFilterType::Anisotropic_8:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 8;
            break;
        case TextureFilterType::Anisotropic_16:
            samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            samplerDescription.MaxAnisotropy = 16;
            break;
        default: ;
        }
        switch (samplerType.wrapMode)
        {
        case TextureWrapMode::Repeat: 
            samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            break;
        case TextureWrapMode::Mirror: 
            samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
            samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
            samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
            break;
        case TextureWrapMode::Clamp: 
            samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            break;
        default: ;
        }
        samplerDescription.MipLODBias = 0.0f;
        samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDescription.BorderColor[0] = 1.0f;
        samplerDescription.BorderColor[1] = 1.0f;
        samplerDescription.BorderColor[2] = 1.0f;
        samplerDescription.BorderColor[3] = 1.0f;
        samplerDescription.MinLOD = -FLT_MAX;
        samplerDescription.MaxLOD = FLT_MAX;
        ID3D11SamplerState* samplerState = nullptr;
        const HRESULT result = m_Device->CreateSamplerState(&samplerDescription, &samplerState);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 sampler state"));
            return nullptr;
        }

        return m_TextureSamplers[samplerType] = samplerState;
    }
}

#endif
