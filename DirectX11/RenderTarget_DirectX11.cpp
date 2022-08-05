// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderTarget_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11.h>
#include <dxgi1_2.h>

#include "RenderEngine_DirectX11.h"
#include "TextureFormat_DirectX11.h"
#include "renderEngine/RenderEngine.h"
#include "renderEngine/window/DirectX11/WindowController_DirectX11.h"

namespace JumaRenderEngine
{
    RenderTarget_DirectX11::~RenderTarget_DirectX11()
    {
        clearDirectX11();
    }

    bool RenderTarget_DirectX11::initInternal()
    {
        if (!Super::initInternal())
        {
            return false;
        }
        if (isWindowRenderTarget())
        {
            if (!initWindowRenderTarget())
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to initialize DirectX11 window render target"));
                return false;
            }
        }
        else if (!initRenderTarget(nullptr))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize DirectX11 render target"));
            return false;
        }
        return true;
    }
    bool RenderTarget_DirectX11::initWindowRenderTarget()
    {
        const window_id windowID = getWindowID();
        const WindowData_DirectX11* windowData = getRenderEngine()->getWindowController()->findWindowData<WindowData_DirectX11>(windowID);
        IDXGISwapChain1* swapchain = windowData != nullptr ? windowData->swapchain : nullptr;
        if (swapchain == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to get DirectX11 swapchain for window {}"), windowID);
            return false;
        }

        ID3D11Texture2D* swapchainImage = nullptr;
        const HRESULT result = swapchain->GetBuffer(0, IID_PPV_ARGS(&swapchainImage));
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to get DirectX11 swapchain image"));
            return false;
        }
        return initRenderTarget(swapchainImage);
    }
    bool RenderTarget_DirectX11::initRenderTarget(ID3D11Texture2D* resultImage)
    {
        RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        ID3D11Device* device = renderEngine->getDevice();

        const math::uvector2 size = getSize();
        const uint8 sampleCount = GetTextureSamplesNumber(getSampleCount());
        const bool useDepth = true;
        const bool resolveEnabled = getSampleCount() != TextureSamples::X1;

        HRESULT result;
        ID3D11Texture2D* colorImage = nullptr;
        ID3D11Texture2D* depthImage = nullptr;
        ID3D11Texture2D* resolveImage = nullptr;
        ID3D11RenderTargetView* colorImageView = nullptr;
        ID3D11DepthStencilView* depthImageView = nullptr;
        ID3D11ShaderResourceView* resultImageView = nullptr;

        if (!resolveEnabled)
        {
            if (resultImage != nullptr)
            {
                colorImage = resultImage;
            }
            else
            {
                D3D11_TEXTURE2D_DESC colorImageDescription{};
                colorImageDescription.Width = size.x;
                colorImageDescription.Height = size.y;
                colorImageDescription.MipLevels = GetMipLevelCountByTextureSize(size);
                colorImageDescription.ArraySize = 1;
                colorImageDescription.Format = GetDirectX11FormatByTextureFormat(getFormat());
                colorImageDescription.SampleDesc.Count = 1;
                colorImageDescription.SampleDesc.Quality = 0;
                colorImageDescription.Usage = D3D11_USAGE_DEFAULT;
                colorImageDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
                colorImageDescription.CPUAccessFlags = 0;
                colorImageDescription.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
                result = device->CreateTexture2D(&colorImageDescription, nullptr, &colorImage);
                if (FAILED(result))
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 color image"));
                    return false;
                }

                D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription{};
                resourceViewDescription.Format = colorImageDescription.Format;
                resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                resourceViewDescription.Texture2D.MostDetailedMip = 0;
                resourceViewDescription.Texture2D.MipLevels = colorImageDescription.MipLevels;
                result = device->CreateShaderResourceView(colorImage, &resourceViewDescription, &resultImageView);
                if (FAILED(result))
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 color image shader view"));
                    colorImage->Release();
                    return false;
                }
            }
        }
        else
        {
            D3D11_TEXTURE2D_DESC colorImageDescription{};
            colorImageDescription.Width = size.x;
            colorImageDescription.Height = size.y;
            colorImageDescription.MipLevels = 1;
            colorImageDescription.ArraySize = 1;
            colorImageDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            colorImageDescription.SampleDesc.Count = sampleCount;
            colorImageDescription.SampleDesc.Quality = 0;
            colorImageDescription.Usage = D3D11_USAGE_DEFAULT;
            colorImageDescription.BindFlags = D3D11_BIND_RENDER_TARGET;
            colorImageDescription.CPUAccessFlags = 0;
            colorImageDescription.MiscFlags = 0;
            result = device->CreateTexture2D(&colorImageDescription, nullptr, &colorImage);
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 color image"));
                return false;
            }
        }
        result = device->CreateRenderTargetView(colorImage, nullptr, &colorImageView);
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 color image view"));
            if (resultImageView != nullptr)
            {
                resultImageView->Release();
            }
            colorImage->Release();
            return false;
        }

        if (useDepth)
        {
            D3D11_TEXTURE2D_DESC depthImageDescription{};
            depthImageDescription.Width = size.x;
            depthImageDescription.Height = size.y;
            depthImageDescription.MipLevels = 1;
            depthImageDescription.ArraySize = 1;
            depthImageDescription.Format = GetDirectX11FormatByTextureFormat(TextureFormat::DEPTH24_STENCIL8);
            depthImageDescription.SampleDesc.Count = sampleCount;
            depthImageDescription.SampleDesc.Quality = 0;
            depthImageDescription.Usage = D3D11_USAGE_DEFAULT;
            depthImageDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            depthImageDescription.CPUAccessFlags = 0;
            depthImageDescription.MiscFlags = 0;
            result = device->CreateTexture2D(&depthImageDescription, nullptr, &depthImage);
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 depth image"));
                if (resultImageView != nullptr)
                {
                    resultImageView->Release();
                }
                colorImageView->Release();
                colorImage->Release();
                return false;
            }
            result = device->CreateDepthStencilView(depthImage, nullptr, &depthImageView);
            if (FAILED(result))
            {
                JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 depth image view"));
                depthImage->Release();
                if (resultImageView != nullptr)
                {
                    resultImageView->Release();
                }
                colorImageView->Release();
                colorImage->Release();
                return false;
            }
        }

        if (resolveEnabled)
        {
            if (resultImage != nullptr)
            {
                resolveImage = resultImage;
            }
            else
            {
                D3D11_TEXTURE2D_DESC resolveImageDescription{};
                resolveImageDescription.Width = size.x;
                resolveImageDescription.Height = size.y;
                resolveImageDescription.MipLevels = GetMipLevelCountByTextureSize(size);
                resolveImageDescription.ArraySize = 1;
                resolveImageDescription.Format = GetDirectX11FormatByTextureFormat(getFormat());
                resolveImageDescription.SampleDesc.Count = 1;
                resolveImageDescription.SampleDesc.Quality = 0;
                resolveImageDescription.Usage = D3D11_USAGE_DEFAULT;
                resolveImageDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
                resolveImageDescription.CPUAccessFlags = 0;
                resolveImageDescription.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
                result = device->CreateTexture2D(&resolveImageDescription, nullptr, &resolveImage);
                if (FAILED(result))
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 resolve image"));
                    if (useDepth)
                    {
                        depthImageView->Release();
                        depthImage->Release();
                    }
                    colorImageView->Release();
                    colorImage->Release();
                    return false;
                }

                D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription{};
                resourceViewDescription.Format = resolveImageDescription.Format;
                resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                resourceViewDescription.Texture2D.MostDetailedMip = 0;
                resourceViewDescription.Texture2D.MipLevels = resolveImageDescription.MipLevels;
                result = device->CreateShaderResourceView(resolveImage, &resourceViewDescription, &resultImageView);
                if (FAILED(result))
                {
                    JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 color image shader view"));
                    resolveImage->Release();
                    if (useDepth)
                    {
                        depthImageView->Release();
                        depthImage->Release();
                    }
                    colorImageView->Release();
                    colorImage->Release();
                    return false;
                }
            }
        }

        ID3D11RasterizerState* rasterizerState = renderEngine->getRasterizerState({ true, false });
        if (rasterizerState == nullptr)
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 rasterizer state"));
            resultImageView->Release();
            depthImageView->Release();
            colorImageView->Release();
            resolveImage->Release();
            depthImage->Release();
            colorImage->Release();
            return false;
        }

        m_ColorAttachmentImage = colorImage;
        m_DepthAttachmentImage = depthImage;
        m_ResolveAttachmentImage = resolveImage;
        m_ColorAttachmentView = colorImageView;
        m_DepthAttachmentView = depthImageView;
        m_ResultImageView = resultImageView;
        m_RasterizerState = rasterizerState;
        return true;
    }

    void RenderTarget_DirectX11::clearDirectX11()
    {
        m_RasterizerState = nullptr;

        clearRenderTarget();
    }
    void RenderTarget_DirectX11::clearRenderTarget()
    {
        if (m_ResultImageView != nullptr)
        {
            m_ResultImageView->Release();
            m_ResultImageView = nullptr;
        }
        if (m_ResolveAttachmentImage != nullptr)
        {
            m_ResolveAttachmentImage->Release();
            m_ResolveAttachmentImage = nullptr;
        }
        if (m_DepthAttachmentView != nullptr)
        {
            m_DepthAttachmentView->Release();
            m_DepthAttachmentImage->Release();
            m_DepthAttachmentView = nullptr;
            m_DepthAttachmentImage = nullptr;
        }
        if (m_ColorAttachmentView != nullptr)
        {
            m_ColorAttachmentView->Release();
            m_ColorAttachmentImage->Release();
            m_ColorAttachmentView = nullptr;
            m_ColorAttachmentImage = nullptr;
        }
    }

    bool RenderTarget_DirectX11::recreateRenderTarget()
    {
        clearRenderTarget();
        return isWindowRenderTarget() ? initWindowRenderTarget() : initRenderTarget(nullptr);
    }

    bool RenderTarget_DirectX11::onStartRender(RenderOptions* renderOptions)
    {
        if (!Super::onStartRender(renderOptions))
        {
            return false;
        }
        if (m_ColorAttachmentView == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid render target"));
            return false;
        }

        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();

        constexpr float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        deviceContext->ClearRenderTargetView(m_ColorAttachmentView, clearColor);
        deviceContext->ClearDepthStencilView(m_DepthAttachmentView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        deviceContext->OMSetRenderTargets(1, &m_ColorAttachmentView, m_DepthAttachmentView);

        const math::uvector2 size = getSize();
        const D3D11_VIEWPORT viewport = { 0, 0, static_cast<FLOAT>(size.x), static_cast<FLOAT>(size.y), 0.0f, 1.0f };
        deviceContext->RSSetState(m_RasterizerState);
        deviceContext->RSSetViewports(1, &viewport);

        return true;
    }
    void RenderTarget_DirectX11::onFinishRender(RenderOptions* renderOptions)
    {
        ID3D11DeviceContext* deviceContext = getRenderEngine<RenderEngine_DirectX11>()->getDeviceContext();
        deviceContext->OMSetRenderTargets(0, nullptr, nullptr);

        if (getSampleCount() != TextureSamples::X1)
        {
            deviceContext->ResolveSubresource(
                m_ResolveAttachmentImage, 0, 
                m_ColorAttachmentImage, 0, 
                GetDirectX11FormatByTextureFormat(getFormat())
            );
        }
        if (!isWindowRenderTarget())
        {
            deviceContext->GenerateMips(m_ResultImageView);
        }

        Super::onFinishRender(renderOptions);
    }
}

#endif
