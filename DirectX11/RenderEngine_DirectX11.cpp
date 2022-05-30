// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderEngine_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11.h>

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

        getWindowController<WindowController_DirectX11>()->clearWindowSwapchains();
        
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
        return nullptr;
    }
    Texture* RenderEngine_DirectX11::createTextureInternal()
    {
        return nullptr;
    }
    Shader* RenderEngine_DirectX11::createShaderInternal()
    {
        return nullptr;
    }
    Material* RenderEngine_DirectX11::createMaterialInternal()
    {
        return nullptr;
    }
    RenderTarget* RenderEngine_DirectX11::createRenderTargetInternal()
    {
        return nullptr;
    }
}

#endif
