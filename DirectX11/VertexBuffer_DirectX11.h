// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/VertexBuffer.h"

struct ID3D11Buffer;

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX11 : public VertexBuffer
    {
    public:
        VertexBuffer_DirectX11() = default;
        virtual ~VertexBuffer_DirectX11() override;

        virtual void render(const RenderOptions* renderOptions, Material* material) override;

    protected:

        virtual bool initInternal(VertexBufferData* verticesData) override;

    private:

        ID3D11Buffer* m_VertexBuffer = nullptr;
        ID3D11Buffer* m_IndexBuffer = nullptr;

        uint32 m_RenderElementsCount = 0;
        uint32 m_VertexSize = 0;


        void clearDirectX();
    };
}

#endif
