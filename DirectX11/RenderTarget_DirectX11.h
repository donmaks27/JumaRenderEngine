// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/RenderTarget.h"

struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11RasterizerState;

namespace JumaRenderEngine
{
    class RenderTarget_DirectX11 : public RenderTarget
    {
        using Super = RenderTarget;

    public:
        RenderTarget_DirectX11() = default;
        virtual ~RenderTarget_DirectX11() override;

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    protected:

        virtual bool initInternal() override;

    private:

        ID3D11Texture2D* m_ColorAttachmentImage = nullptr;
        ID3D11Texture2D* m_DepthAttachmentImage = nullptr;
        ID3D11Texture2D* m_ResolveAttachmentImage = nullptr;
        ID3D11RenderTargetView* m_ColorAttachmentView = nullptr;
        ID3D11DepthStencilView* m_DepthAttachmentView = nullptr;
        ID3D11ShaderResourceView* m_ResultImageView = nullptr;

        ID3D11RasterizerState* m_RasterizerState = nullptr;


        bool initWindowRenderTarget();
        bool initRenderTarget(ID3D11Texture2D* resultImage);

        void clearDirectX11();
    };
}

#endif
