// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/Material.h"

struct ID3D11Buffer;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilState;

namespace JumaRenderEngine
{
    class VertexBuffer_DirectX11;
    struct RenderOptions;

    class Material_DirectX11 : public Material
    {
        using Super = Material;

    public:
        Material_DirectX11() = default;
        virtual ~Material_DirectX11() override;

        bool bindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);
        void unbindMaterial(const RenderOptions* renderOptions, VertexBuffer_DirectX11* vertexBuffer);

    protected:

        virtual bool initInternal() override;

    private:

        struct UniformBufferDescription
        {
            ID3D11Buffer* buffer = nullptr;
            uint8 shaderStages = 0;
        };

        jmap<uint32, UniformBufferDescription> m_UniformBuffers;
        ID3D11DepthStencilState* m_DepthStencilState = nullptr;


        void clearDirectX();

        void updateUniformBuffersData(ID3D11DeviceContext* deviceContext);
    };
}

#endif
