// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/VertexBuffer.h"

namespace JumaRenderEngine
{
    class DirectX12Buffer;

    class VertexBuffer_DirectX12 : public VertexBuffer
    {
    public:
        VertexBuffer_DirectX12() = default;
        virtual ~VertexBuffer_DirectX12() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(VertexBufferData* verticesData) override;

    private:

        DirectX12Buffer* m_VertexBuffer = nullptr;
        DirectX12Buffer* m_IndexBuffer = nullptr;

        uint32 m_CachedVertexSize = 0;
        uint32 m_RenderElementsCount = 0;


        void clearDirectX();
    };
}

#endif
