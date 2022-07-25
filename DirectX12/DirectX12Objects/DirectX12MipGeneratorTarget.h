// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

struct ID3D12DescriptorHeap;

namespace JumaRenderEngine
{
    class RenderEngineContextObjectBase;
    class DirectX12Texture;
    class DirectX12MipGenerator;

    class DirectX12MipGeneratorTarget
    {
        friend DirectX12MipGenerator;

    public:
        DirectX12MipGeneratorTarget() = default;
        virtual ~DirectX12MipGeneratorTarget() = default;

    protected:

        virtual DirectX12Texture* getMipGeneratorTargetTexture() const = 0;

        bool initMipGeneratorTarget();
        void clearMipGeneratorTarget();

    private:

        DirectX12Texture* m_StagingTexture = nullptr;
        ID3D12DescriptorHeap* m_DescriptorHeap_SRV_UAV = nullptr;
        uint8 m_GeneratingStagesCount = 0;
    };
}

#endif
