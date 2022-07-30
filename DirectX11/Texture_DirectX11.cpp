// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Texture_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include <d3d11.h>

#include "RenderEngine_DirectX11.h"
#include "TextureFormat_DirectX11.h"

namespace JumaRenderEngine
{
    Texture_DirectX11::~Texture_DirectX11()
    {
        clearDirectX();
    }

    bool Texture_DirectX11::initInternal(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        RenderEngine_DirectX11* renderEngine = getRenderEngine<RenderEngine_DirectX11>();
        ID3D11Device* device = renderEngine->getDevice();

        D3D11_TEXTURE2D_DESC textureDescription{};
        textureDescription.Width = size.x;
        textureDescription.Height = size.y;
        textureDescription.MipLevels = GetMipLevelCountByTextureSize(size);
        textureDescription.ArraySize = 1;
        textureDescription.Format = GetDirectX11FormatByTextureFormat(format);
        textureDescription.SampleDesc.Count = 1;
        textureDescription.SampleDesc.Quality = 0;
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDescription.CPUAccessFlags = 0;
        textureDescription.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        ID3D11Texture2D* texture = nullptr;
        HRESULT result = device->CreateTexture2D(&textureDescription, nullptr, &texture);
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 texture"));
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDescription{};
        textureViewDescription.Format = textureDescription.Format;
        textureViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        textureViewDescription.Texture2D.MostDetailedMip = 0;
        textureViewDescription.Texture2D.MipLevels = textureDescription.MipLevels;
        ID3D11ShaderResourceView* textureView = nullptr;
        result = device->CreateShaderResourceView(texture, &textureViewDescription, &textureView);
        if (FAILED(result))
        {
            JUMA_RENDER_ERROR_LOG(result, JSTR("Failed to create DirectX11 texture shader resource view"));
            texture->Release();
            return false;
        }

        ID3D11DeviceContext* deviceContext = renderEngine->getDeviceContext();
        const D3D11_BOX textureRange = { 0, 0, 0, size.x, size.y, 1 };
        deviceContext->UpdateSubresource(texture, 0, &textureRange, data, size.x * GetTextureFormatSize(format), 1);
        deviceContext->GenerateMips(textureView);

        m_Texture = texture;
        m_TextureView = textureView;
        return true;
    }

    void Texture_DirectX11::clearDirectX()
    {
        if (m_TextureView != nullptr)
        {
            m_TextureView->Release();
            m_TextureView = nullptr;
        }
        if (m_Texture != nullptr)
        {
            m_Texture->Release();
            m_Texture = nullptr;
        }
    }
}

#endif
