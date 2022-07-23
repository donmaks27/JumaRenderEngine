// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderTarget_DirectX12.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "RenderEngine_DirectX12.h"
#include "RenderOptions_DirectX12.h"
#include "TextureFormat_DirectX12.h"
#include "DirectX12Objects/DirectX12Swapchain.h"
#include "DirectX12Objects/DirectX12Texture.h"
#include "renderEngine/window/DirectX12/WindowController_DirectX12.h"

namespace JumaRenderEngine
{
    RenderTarget_DirectX12::~RenderTarget_DirectX12()
    {
        clearDirectX();
    }

    bool RenderTarget_DirectX12::initInternal()
    {
        if (!(isWindowRenderTarget() ? initWindowRenderTarget() : initRenderTarget()))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to init DirectX12 render target"));
            return false;
        }
        return true;
    }
    bool RenderTarget_DirectX12::initWindowRenderTarget()
    {
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const window_id windowID = getWindowID();
        const WindowData_DirectX12* windowData = renderEngine->getWindowController()->findWindowData<WindowData_DirectX12>(windowID);
        const DirectX12Swapchain* swapchain = windowData != nullptr ? windowData->swapchain : nullptr;
        if (swapchain == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to find swapchain for window ") + TO_JSTR(windowID));
            return false;
        }

        ID3D12Device2* device = renderEngine->getDevice();
        const TextureSamples samples = getSampleCount();
        const uint8 samplesCount = GetTextureSamplesNumber(samples);
        const math::uvector2 size = windowData->properties.size;

        const uint8 buffersCount = swapchain->getBuffersCount();
        jarray<DirectX12Texture*> buffers(buffersCount, nullptr);
        for (uint8 bufferIndex = 0; bufferIndex < buffersCount; bufferIndex++)
        {
            buffers[bufferIndex] = swapchain->getBuffer(bufferIndex);
        }

        DirectX12Texture* renderTexture = nullptr;
        ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
        if (samplesCount > 1)
        {
            renderTexture = renderEngine->createObject<DirectX12Texture>();
            renderTexture->initColor(
                size, samplesCount, GetDirectX12FormatByTextureFormat(getFormat()), 1, D3D12_RESOURCE_STATE_RENDER_TARGET, 
                D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
            );
            if (!renderTexture->isValid())
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create render texture"));
                delete renderTexture;
                return false;
            }

            rtvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
            if (rtvDescriptorHeap == nullptr)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create RTV descriptor for render texture"));
                delete renderTexture;
                return false;
            }
            device->CreateRenderTargetView(renderTexture->getResource(), nullptr, rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        }
        else
        {
            rtvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, buffersCount, false);
            if (rtvDescriptorHeap == nullptr)
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create RTV descriptor for render texture"));
                return false;
            }
            for (uint8 bufferIndex = 0; bufferIndex < buffersCount; bufferIndex++)
            {
                device->CreateRenderTargetView(
                    buffers[bufferIndex]->getResource(), nullptr, 
                    renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>(rtvDescriptorHeap, bufferIndex)
                );
            }
        }

        DirectX12Texture* depthTexture = renderEngine->createObject<DirectX12Texture>();
        depthTexture->initDepth(
            size, samplesCount, GetDirectX12FormatByTextureFormat(TextureFormat::DEPTH_UNORM24_STENCIL_UINT8), D3D12_RESOURCE_STATE_DEPTH_WRITE, 
            D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
        );
        if (!depthTexture->isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create depth texture"));
            delete depthTexture;
            rtvDescriptorHeap->Release();
            delete renderTexture;
            return false;
        }

        ID3D12DescriptorHeap* dsvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
        if (dsvDescriptorHeap == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create descriptor heap for DSV"));
            delete depthTexture;
            rtvDescriptorHeap->Release();
            delete renderTexture;
            return false;
        }
        device->CreateDepthStencilView(depthTexture->getResource(), nullptr, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        m_ColorTexture = renderTexture;
        m_ResultTextures = buffers;
        m_DescriptorHeapRTV = rtvDescriptorHeap;
        m_DepthTexture = depthTexture;
        m_DescriptorHeapDSV = dsvDescriptorHeap;
        return true;
    }
    bool RenderTarget_DirectX12::initRenderTarget()
    {
        RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        ID3D12Device2* device = renderEngine->getDevice();

        const DXGI_FORMAT format = GetDirectX12FormatByTextureFormat(getFormat());
        const TextureSamples samples = getSampleCount();
        const uint8 samplesCount = GetTextureSamplesNumber(samples);
        const bool sholdResolve = samplesCount > 1;
        const math::uvector2 size = getSize();

        DirectX12Texture* renderTexture = renderEngine->createObject<DirectX12Texture>();
        renderTexture->initColor(
            size, samplesCount, format, 1, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );
        if (!renderTexture->isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create render texture"));
            delete renderTexture;
            return false;
        }

        DirectX12Texture* depthTexture = renderEngine->createObject<DirectX12Texture>();
        depthTexture->initDepth(
            size, samplesCount, GetDirectX12FormatByTextureFormat(TextureFormat::DEPTH_UNORM24_STENCIL_UINT8), D3D12_RESOURCE_STATE_DEPTH_WRITE, 
            D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
        );
        if (!depthTexture->isValid())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create depth texture"));
            delete depthTexture;
            delete renderTexture;
            return false;
        }

        DirectX12Texture* resolveTexture = nullptr;
        if (sholdResolve)
        {
            resolveTexture = renderEngine->createObject<DirectX12Texture>();
            resolveTexture->initColor(size, 1, format, 0, D3D12_RESOURCE_STATE_RESOLVE_DEST);
            if (!resolveTexture->isValid())
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create resolve texture"));
                delete resolveTexture;
                delete depthTexture;
                delete renderTexture;
                return false;
            }
        }

        ID3D12DescriptorHeap* rtvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 1, false);
        if (rtvDescriptorHeap == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create RTV descriptor heap"));
            delete resolveTexture;
            delete depthTexture;
            delete renderTexture;
            return false;
        }
        ID3D12DescriptorHeap* dsvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
        if (dsvDescriptorHeap == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create descriptor heap for DSV"));
            rtvDescriptorHeap->Release();
            delete resolveTexture;
            delete depthTexture;
            delete renderTexture;
            return false;
        }
        ID3D12DescriptorHeap* srvDescriptorHeap = renderEngine->createDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, false);
        if (srvDescriptorHeap == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to create descriptor heap for SRV"));
            dsvDescriptorHeap->Release();
            rtvDescriptorHeap->Release();
            delete resolveTexture;
            delete depthTexture;
            delete renderTexture;
            return false;
        }

        device->CreateRenderTargetView(renderTexture->getResource(), nullptr, rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        device->CreateDepthStencilView(depthTexture->getResource(), nullptr, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
        device->CreateShaderResourceView(resolveTexture != nullptr ? resolveTexture->getResource() : renderTexture->getResource(), nullptr, srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

        m_ColorTexture = renderTexture;
        if (resolveTexture != nullptr)
        {
            m_ResultTextures = { resolveTexture };
        }
        m_DescriptorHeapRTV = rtvDescriptorHeap;
        m_DescriptorHeapSRV = srvDescriptorHeap;
        m_DepthTexture = depthTexture;
        m_DescriptorHeapDSV = dsvDescriptorHeap;
        return true;
    }

    void RenderTarget_DirectX12::clearDirectX()
    {
        if (m_DescriptorHeapDSV != nullptr)
        {
            m_DescriptorHeapDSV->Release();
            m_DescriptorHeapDSV = nullptr;
        }
        if (m_DescriptorHeapSRV != nullptr)
        {
            m_DescriptorHeapSRV->Release();
            m_DescriptorHeapSRV = nullptr;
        }
        if (m_DescriptorHeapRTV != nullptr)
        {
            m_DescriptorHeapRTV->Release();
            m_DescriptorHeapRTV = nullptr;
        }

        if (m_ColorTexture != nullptr)
        {
            delete m_ColorTexture;
            m_ColorTexture = nullptr;
        }
        if (m_DepthTexture != nullptr)
        {
            delete m_DepthTexture;
            m_DepthTexture = nullptr;
        }
        if (!isWindowRenderTarget())
        {
            for (const auto& texture : m_ResultTextures)
            {
                delete texture;
            }
        }
        m_ResultTextures.clear();
    }

    bool RenderTarget_DirectX12::onStartRender(RenderOptions* renderOptions)
    {
        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }

        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();

        math::uvector2 size;
        DirectX12Texture* renderTexture = nullptr;
        uint8 rtvIndex = 0;
        if (isWindowRenderTarget())
        {
            const WindowData_DirectX12* windowData = renderEngine->getWindowController()->findWindowData<WindowData_DirectX12>(getWindowID());
            if (windowData == nullptr)
            {
                return false;
            }

            size = windowData->properties.size;
            if (getSampleCount() == TextureSamples::X1)
            {
                rtvIndex = windowData->swapchain->getCurrentBufferIndex();
                renderTexture = m_ResultTextures[rtvIndex];
            }
            else
            {
                rtvIndex = 0;
                renderTexture = m_ColorTexture;
            }
        }
        else
        {
            size = getSize();
            rtvIndex = 0;
            renderTexture = m_ColorTexture;
        }

        DirectX12CommandList* commandListObject = reinterpret_cast<RenderOptions_DirectX12*>(renderOptions)->renderCommandList;
        ID3D12GraphicsCommandList2* commandList = commandListObject->get();

        commandListObject->changeTextureState(renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
        commandListObject->applyTextureStateChanges();

        const D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor = renderEngine->getDescriptorCPU<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>(m_DescriptorHeapRTV, rtvIndex);
        const D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptor = m_DescriptorHeapDSV->GetCPUDescriptorHandleForHeapStart();
        static constexpr FLOAT clearColor[] = { 0.0f, 1.0f, 1.0f, 1.0f };
        static constexpr FLOAT clearColorW[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        commandList->ClearRenderTargetView(rtvDescriptor, !isWindowRenderTarget() ? clearColor : clearColorW, 0, nullptr);
        commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
        commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);

        const D3D12_VIEWPORT viewport = {
            0.0f, 0.0f, static_cast<FLOAT>(size.x), static_cast<FLOAT>(size.y), 0.0f, 1.0f
        };
        constexpr D3D12_RECT scissors = { 0, 0, LONG_MAX, LONG_MAX };
        commandList->RSSetViewports(1, &viewport);
        commandList->RSSetScissorRects(1, &scissors);
        return true;
    }

    void RenderTarget_DirectX12::onFinishRender(RenderOptions* renderOptions)
    {
        const RenderEngine_DirectX12* renderEngine = getRenderEngine<RenderEngine_DirectX12>();
        const bool shouldResolve = getSampleCount() != TextureSamples::X1;

        DirectX12Texture* renderTexture = nullptr;
        DirectX12Texture* resolveTexture = nullptr;
        if (isWindowRenderTarget())
        {
            const WindowData_DirectX12* windowData = renderEngine->getWindowController()->findWindowData<WindowData_DirectX12>(getWindowID());
            if (!shouldResolve)
            {
                renderTexture = m_ResultTextures[windowData->swapchain->getCurrentBufferIndex()];
            }
            else
            {
                renderTexture = m_ColorTexture;
                resolveTexture = m_ResultTextures[windowData->swapchain->getCurrentBufferIndex()];
            }
        }
        else
        {
            renderTexture = m_ColorTexture;
            if (shouldResolve)
            {
                resolveTexture = m_ResultTextures[0];
            }
        }

        DirectX12CommandList* commandListObject = reinterpret_cast<RenderOptions_DirectX12*>(renderOptions)->renderCommandList;
        ID3D12GraphicsCommandList2* commandList = commandListObject->get();

        if (!shouldResolve)
        {
            commandListObject->changeTextureState(renderTexture, isWindowRenderTarget() ? D3D12_RESOURCE_STATE_PRESENT : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            commandListObject->applyTextureStateChanges();
        }
        else
        {
            commandListObject->changeTextureState(renderTexture, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
            commandListObject->changeTextureState(resolveTexture, D3D12_RESOURCE_STATE_RESOLVE_DEST);
            commandListObject->applyTextureStateChanges();

            commandList->ResolveSubresource(
                resolveTexture->getResource(), 0, renderTexture->getResource(), 0, 
                GetDirectX12FormatByTextureFormat(getFormat())
            );

            commandListObject->changeTextureState(renderTexture, D3D12_RESOURCE_STATE_RENDER_TARGET);
            commandListObject->changeTextureState(
                resolveTexture, isWindowRenderTarget() ? D3D12_RESOURCE_STATE_PRESENT : D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
            );
            commandListObject->applyTextureStateChanges();
        }

        Super::onFinishRender(renderOptions);
    }
}

#endif
