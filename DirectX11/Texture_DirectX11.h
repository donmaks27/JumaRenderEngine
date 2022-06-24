// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/Texture.h"

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace JumaRenderEngine
{
    class Texture_DirectX11 : public Texture
    {
    public:
        Texture_DirectX11() = default;
        virtual ~Texture_DirectX11() override;

        ID3D11ShaderResourceView* getTextureView() const { return m_TextureView; }

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) override;

    private:

        ID3D11Texture2D* m_Texture = nullptr;
        ID3D11ShaderResourceView* m_TextureView = nullptr;


        void clearDirectX();
    };
}

#endif
