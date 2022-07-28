﻿// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11_1.h>
#include <dxgi1_3.h>

#include "renderEngine/DirectX11/RenderEngine_DirectX11.h"
#include "renderEngine/DirectX11/RenderTarget_DirectX11.h"
#include "renderEngine/DirectX11/TextureFormat_DirectX11.h"

namespace JumaRenderEngine
{
    WindowController_DirectX11::~WindowController_DirectX11()
    {
        clearDirectX11();
    }

    void WindowController_DirectX11::clearDirectX11()
    {
    }

    void WindowController_DirectX11::clearWindowDirectX11(const window_id windowID, WindowData_DirectX11& windowData)
    {
        clearWindow(windowID, windowData);

        destroyWindowSwapchain(windowID, windowData);

        windowData.windowHandler = nullptr;
    }

    bool WindowController_DirectX11::createWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            if (!createWindowSwapchain(windowID, *getWindowData<WindowData_DirectX11>(windowID)))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX11 swapchain"));
                return false;
            }
        }
        return true;
    }
    void WindowController_DirectX11::clearWindowSwapchains()
    {
        for (const auto& windowID : getWindowIDs())
        {
            destroyWindowSwapchain(windowID, *getWindowData<WindowData_DirectX11>(windowID));
        }
    }

    bool WindowController_DirectX11::createWindowSwapchain(const window_id windowID, WindowData_DirectX11& windowData)
    {
        ID3D11Device* device = getRenderEngine<RenderEngine_DirectX11>()->getDevice();
        if (device == nullptr)
        {
            // DirectX11 device is not created yet
            return true;
        }

        IDXGIFactory3* factory = nullptr;
#if defined(JDEBUG)
        constexpr UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
        constexpr UINT createFactoryFlags = 0;
#endif
        HRESULT result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DXGIFactory3"));
            return false;
        }

        constexpr uint8 buffersCount = 3;
        DXGI_SWAP_CHAIN_DESC1 swapchainDescription{};
        swapchainDescription.Width = windowData.properties.size.x;
        swapchainDescription.Height = windowData.properties.size.y;
        swapchainDescription.Format = GetDirectX11FormatByTextureFormat(TextureFormat::RGBA8);
        swapchainDescription.Stereo = FALSE;
        swapchainDescription.SampleDesc.Count = 1;
        swapchainDescription.SampleDesc.Quality = 0;
        swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDescription.BufferCount = buffersCount;
        swapchainDescription.Scaling = DXGI_SCALING_STRETCH;
        swapchainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchainDescription.Flags = 0; // TODO: Tearing supported?
        result = factory->CreateSwapChainForHwnd(device, windowData.windowHandler, &swapchainDescription, nullptr, nullptr, &windowData.swapchain);
        factory->Release();
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 swapchain"));
            return false;
        }
        return true;
    }
    void WindowController_DirectX11::destroyWindowSwapchain(const window_id windowID, WindowData_DirectX11& windowData)
    {
        if (windowData.swapchain != nullptr)
        {
            windowData.swapchain->Release();
            windowData.swapchain = nullptr;
        }
    }

    void WindowController_DirectX11::onWindowResized(const window_id windowID, const math::uvector2& newSize)
    {
        WindowData_DirectX11* windowData = getWindowData<WindowData_DirectX11>(windowID);
        windowData->properties.size = newSize;

        RenderTarget_DirectX11* renderTarget = dynamic_cast<RenderTarget_DirectX11*>(windowData->windowRenderTarget);
        if (renderTarget != nullptr)
        {
            renderTarget->clearRenderTarget();
        }
        constexpr uint8 buffersCount = 3;
        windowData->swapchain->ResizeBuffers(buffersCount, newSize.x, newSize.y, GetDirectX11FormatByTextureFormat(TextureFormat::RGBA8), 0);
        
        OnWindowPropertiesChanged.call(this, windowData);
    }

    void WindowController_DirectX11::onFinishWindowRender(const window_id windowID)
    {
        const WindowData_DirectX11* windowData = findWindowData<WindowData_DirectX11>(windowID);
        if (windowData->swapchain != nullptr)
        {
            windowData->swapchain->Present(1, 0);
        }

        Super::onFinishWindowRender(windowID);
    }
}

#endif
