// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/RenderTarget.h"

#include "jutils/jarray.h"

struct ID3D12DescriptorHeap;

namespace JumaRenderEngine
{
    class DirectX12Texture;

    class RenderTarget_DirectX12 : public RenderTarget
    {
        using Super = RenderTarget;

    public:
        RenderTarget_DirectX12() = default;
        virtual ~RenderTarget_DirectX12() override;

        virtual bool onStartRender(RenderOptions* renderOptions) override;
        virtual void onFinishRender(RenderOptions* renderOptions) override;

    protected:

        virtual bool initInternal() override;

    private:

        DirectX12Texture* m_ColorTexture = nullptr;
        jarray<DirectX12Texture*> m_ResultTextures;
        ID3D12DescriptorHeap* m_DescriptorHeapRTV = nullptr;

        DirectX12Texture* m_DepthTexture = nullptr;
        ID3D12DescriptorHeap* m_DescriptorHeapDSV = nullptr;


        bool initWindowRenderTarget();
        bool initRenderTarget();

        void clearDirectX();
    };
}

#endif
