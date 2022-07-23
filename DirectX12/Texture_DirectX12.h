// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/Texture.h"

struct ID3D12DescriptorHeap;

namespace JumaRenderEngine
{
    class DirectX12Texture;

    class Texture_DirectX12 : public Texture
    {
    public:
        Texture_DirectX12() = default;
        virtual ~Texture_DirectX12() override;

        DirectX12Texture* getTextureObject() const { return m_Texture; }
        ID3D12DescriptorHeap* getSRV() const { return m_DescriptorHeapSRV; }

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) override;

    private:

        DirectX12Texture* m_Texture = nullptr;
        ID3D12DescriptorHeap* m_DescriptorHeapSRV = nullptr;


        void clearDirectX();
    };
}

#endif
