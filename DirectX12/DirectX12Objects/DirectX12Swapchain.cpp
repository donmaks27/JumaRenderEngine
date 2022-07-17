// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "DirectX12Swapchain.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <dxgi1_5.h>

#include "DirectX12Texture.h"
#include "renderEngine/DirectX12/RenderEngine_DirectX12.h"
#include "renderEngine/window/DirectX12/WindowController_DirectX12.h"

namespace JumaRenderEngine
{
    DirectX12Swapchain::~DirectX12Swapchain()
    {
        clearDirectX();
    }

    bool DirectX12Swapchain::init(const window_id windowID)
    {
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const WindowController_DirectX12* windowController = renderEngine->getWindowController<WindowController_DirectX12>();
        const WindowData_DirectX12* windowData = windowController->findWindowData<WindowData_DirectX12>(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Can't find window ") + TO_JSTR(windowID));
            return false;
        }

        const DirectX12CommandQueue* commandQueue = renderEngine->getCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (commandQueue == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get command queue"));
            return false;
        }

        IDXGIFactory4* factory = nullptr;
#if defined(JDEBUG)
        constexpr UINT createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
        constexpr UINT createFactoryFlags = 0;
#endif
        HRESULT result = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&factory));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DXGIFactory2"));
            return false;
        }

        constexpr uint8 buffersCount = 3;
        DXGI_SWAP_CHAIN_DESC1 swapchainDescription{};
        swapchainDescription.Width = windowData->properties.size.x;
        swapchainDescription.Height = windowData->properties.size.y;
        swapchainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchainDescription.Stereo = FALSE;
        swapchainDescription.SampleDesc.Count = 1;
        swapchainDescription.SampleDesc.Quality = 0;
        swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchainDescription.BufferCount = buffersCount;
        swapchainDescription.Scaling = DXGI_SCALING_STRETCH;
        swapchainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchainDescription.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchainDescription.Flags = windowController->isTearingSupported() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        IDXGISwapChain1* swapchain1 = nullptr;
        result = factory->CreateSwapChainForHwnd(commandQueue->get(), windowData->windowHandler, &swapchainDescription, nullptr, nullptr, &swapchain1);
        factory->Release();
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX12 swapchain"));
            return false;
        }

        IDXGISwapChain4* swapchain4 = nullptr;
        swapchain1->QueryInterface(&swapchain4);
        swapchain1->Release();
        if (swapchain4 == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create DirectX12 swapchain"));
            return false;
        }

        m_SwapchainBuffers.reserve(buffersCount);
        for (uint8 bufferIndex = 0; bufferIndex < buffersCount; bufferIndex++)
        {
            ID3D12Resource* swapchainBuffer = nullptr;
            if (FAILED(swapchain4->GetBuffer(bufferIndex, IID_PPV_ARGS(&swapchainBuffer))))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to get DirectX12 swapchain buffer"));
                for (const auto& buffer : m_SwapchainBuffers)
                {
                    buffer.getResource()->Release();
                }
                m_SwapchainBuffers.clear();
                swapchain4->Release();
                return false;
            }
            renderEngine->registerObject(&m_SwapchainBuffers.addDefault())->init(swapchainBuffer, D3D12_RESOURCE_STATE_PRESENT);
        }

        m_Swapchain = swapchain4;
        m_CurrentBufferIndex = static_cast<uint8>(m_Swapchain->GetCurrentBackBufferIndex());
        markAsInitialized();
        return true;
    }

    void DirectX12Swapchain::clearDirectX()
    {
        for (const auto& buffer : m_SwapchainBuffers)
        {
            buffer.getResource()->Release();
        }
        m_SwapchainBuffers.clear();

        if (m_Swapchain != nullptr)
        {
            m_Swapchain->Release();
            m_Swapchain = nullptr;
        }

        m_CurrentBufferIndex = 0;
    }

    bool DirectX12Swapchain::present()
    {
        const bool vsyncEnabled = true;
        const bool tearingSupported = getRenderEngine()->getWindowController<WindowController_DirectX12>()->isTearingSupported();

        const UINT syncInterval = vsyncEnabled ? 1 : 0;
        const UINT presentFlags = tearingSupported && !vsyncEnabled ? DXGI_PRESENT_ALLOW_TEARING : 0;
        const HRESULT result = m_Swapchain->Present(syncInterval, presentFlags);
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to present swapchain image"));
            return false;
        }

        m_CurrentBufferIndex = static_cast<uint8>(m_Swapchain->GetCurrentBackBufferIndex());
        return true;
    }
}

#endif
