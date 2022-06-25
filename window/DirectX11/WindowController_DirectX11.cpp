// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "WindowController_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11_1.h>
#include <dxgi.h>

#include "renderEngine/DirectX11/RenderEngine_DirectX11.h"

namespace JumaRenderEngine
{
    bool QueryDisplayModeRefreshRate(WindowData_DirectX11& windowData, IDXGIFactory1* factory, DXGI_RATIONAL& outRefreshRate)
    {
        IDXGIAdapter1* adapter;
        HRESULT result = factory->EnumAdapters1(0, &adapter);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to get DirectX11 Adapter1"));
            return false;
        }
        IDXGIOutput* output;
        result = adapter->EnumOutputs(0, &output);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to get DirectX11 Output"));
            adapter->Release();
            return false;
        }
        IDXGIOutput1* output1;
        output->QueryInterface(&output1);

        UINT modeCount = 0;
        output1->GetDisplayModeList1(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_ENUM_MODES_INTERLACED, &modeCount, nullptr);
        jarray<DXGI_MODE_DESC1> modes(static_cast<int32>(modeCount));
        output1->GetDisplayModeList1(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_ENUM_MODES_INTERLACED, &modeCount, modes.getData());

        bool found = false;
        for (const auto& mode : modes)
        {
            if ((mode.Width == windowData.size.x) && (mode.Height == windowData.size.y))
            {
                found = true;
                outRefreshRate = mode.RefreshRate;
            }
        }

        output1->Release();
        output->Release();
        adapter->Release();
        return found;
    }

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

        IDXGIFactory1* factory;
        HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to get DirectX11 Factory1"));
            return false;
        }
        DXGI_RATIONAL refreshRate;
        if (!QueryDisplayModeRefreshRate(windowData, factory, refreshRate))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get refresh rate"));
            factory->Release();
            return false;
        }

        DXGI_SWAP_CHAIN_DESC swapchainDescription{};
        swapchainDescription.BufferCount = 2;
        swapchainDescription.BufferDesc.Width = windowData.size.x;
        swapchainDescription.BufferDesc.Height = windowData.size.y;
        swapchainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchainDescription.BufferDesc.RefreshRate = refreshRate;
        swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDescription.OutputWindow = windowData.windowHandler;
        swapchainDescription.SampleDesc.Count = 1;
        swapchainDescription.SampleDesc.Quality = 0;
        swapchainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDescription.Windowed = TRUE;
        result = factory->CreateSwapChain(device, &swapchainDescription, &windowData.swapchain);
        if (result < 0)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 Swapchain"));
            factory->Release();
            return false;
        }

        factory->Release();
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
