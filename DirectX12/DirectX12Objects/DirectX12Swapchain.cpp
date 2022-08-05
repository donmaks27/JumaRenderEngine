// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "DirectX12Swapchain.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include <dxgi1_5.h>

#include "DirectX12Texture.h"
#include "renderEngine/DirectX12/RenderEngine_DirectX12.h"
#include "renderEngine/DirectX12/RenderTarget_DirectX12.h"
#include "renderEngine/window/DirectX12/WindowController_DirectX12.h"

namespace JumaRenderEngine
{
    DirectX12Swapchain::~DirectX12Swapchain()
    {
        clearDirectX();
    }

    bool DirectX12Swapchain::init(const window_id windowID)
    {
        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        WindowController_DirectX12* windowController = renderEngine->getWindowController<WindowController_DirectX12>();
        const WindowData_DirectX12* windowData = windowController->findWindowData<WindowData_DirectX12>(windowID);
        if (windowData == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Can't find window {}"), windowID);
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

        m_WindowID = windowID;
        m_Swapchain = swapchain4;
        m_SwapchainBuffersSize = windowData->properties.size;
        if (!getSwapchainBuffers(buffersCount))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get swapchain buffers"));
            clearDirectX();
            return false;
        }

        m_SwapchainInvalid = false;

        windowController->OnWindowPropertiesChanged.bind(this, &DirectX12Swapchain::onWindowPropertiesChanged);
        return true;
    }
    bool DirectX12Swapchain::getSwapchainBuffers(const uint8 buffersCount)
    {
        RenderEngine* renderEngine = getRenderEngine();

        m_SwapchainBuffers.reserve(buffersCount);
        for (uint8 bufferIndex = 0; bufferIndex < buffersCount; bufferIndex++)
        {
            ID3D12Resource* swapchainBuffer = nullptr;
            const HRESULT result = m_Swapchain->GetBuffer(bufferIndex, IID_PPV_ARGS(&swapchainBuffer));
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to get DirectX12 swapchain buffer"));
                return false;
            }
            renderEngine->registerObject(&m_SwapchainBuffers.addDefault())->init(swapchainBuffer, D3D12_RESOURCE_STATE_PRESENT);
        }

        m_CurrentBufferIndex = static_cast<uint8>(m_Swapchain->GetCurrentBackBufferIndex());
        return true;
    }

    void DirectX12Swapchain::clearDirectX()
    {
        getRenderEngine()->getWindowController()->OnWindowPropertiesChanged.unbind(this, &DirectX12Swapchain::onWindowPropertiesChanged);

        clearSwapchainBuffers();

        if (m_Swapchain != nullptr)
        {
            m_Swapchain->Release();
            m_Swapchain = nullptr;
        }

        m_WindowID = window_id_INVALID;
        m_SwapchainBuffersSize = { 0, 0 };
        m_CurrentBufferIndex = 0;
        m_SwapchainInvalid = true;
    }
    void DirectX12Swapchain::clearSwapchainBuffers()
    {
        for (const auto& buffer : m_SwapchainBuffers)
        {
            buffer.getResource()->Release();
        }
        m_SwapchainBuffers.clear();
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

    void DirectX12Swapchain::onWindowPropertiesChanged(WindowController* windowController, const WindowData* windowData)
    {
        if (windowData->windowID == getWindowID())
        {
            if (windowData->properties.size != m_SwapchainBuffersSize)
            {
                invalidate();
            }
        }
    }

    bool DirectX12Swapchain::updateSwapchain()
    {
        if (m_SwapchainInvalid)
        {
            const WindowData_DirectX12* windowData = getRenderEngine()->getWindowController()->findWindowData<WindowData_DirectX12>(getWindowID());
            RenderTarget_DirectX12* renderTarget = windowData != nullptr ? dynamic_cast<RenderTarget_DirectX12*>(windowData->windowRenderTarget) : nullptr;
            if (renderTarget == nullptr)
            {
                JUMA_RENDER_LOG(error, JSTR("Can't find render target of window {}"), getWindowID());
                clearDirectX();
                return false;
            }
            renderTarget->clearRenderTarget();

            clearSwapchainBuffers();
            DXGI_SWAP_CHAIN_DESC1 swapchainDescription{};
            m_Swapchain->GetDesc1(&swapchainDescription);
            const HRESULT result = m_Swapchain->ResizeBuffers(0, windowData->properties.size.x, windowData->properties.size.y, DXGI_FORMAT_UNKNOWN, swapchainDescription.Flags);
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to resize swapchain"));
                clearDirectX();
                return false;
            }
            m_SwapchainBuffersSize = windowData->properties.size;
            if (!getSwapchainBuffers(static_cast<uint8>(swapchainDescription.BufferCount)))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to update swapchain buffers"));
                clearDirectX();
                return false;
            }

            renderTarget->invalidate();
            if (!renderTarget->update())
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to update window render target"));
                clearDirectX();
                return false;
            }

            m_SwapchainInvalid = false;
        }
        return true;
    }
}

#endif
