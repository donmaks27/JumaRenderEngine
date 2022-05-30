// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderEngine_DirectX11.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX11)

#include "renderEngine/window/DirectX11/WindowControllerInfo_DirectX11.h"

namespace JumaRenderEngine
{
    RenderEngine_DirectX11::~RenderEngine_DirectX11()
    {
        clearDirectX();
    }

    void RenderEngine_DirectX11::clearInternal()
    {
        clearDirectX();
        Super::clearInternal();
    }
    void RenderEngine_DirectX11::clearDirectX()
    {
    }

    WindowController* RenderEngine_DirectX11::createWindowController()
    {
        return registerObject(WindowControllerInfo<RenderAPI::DirectX11>::create());
    }
    VertexBuffer* RenderEngine_DirectX11::createVertexBufferInternal()
    {
        return nullptr;
    }
    Texture* RenderEngine_DirectX11::createTextureInternal()
    {
        return nullptr;
    }
    Shader* RenderEngine_DirectX11::createShaderInternal()
    {
        return nullptr;
    }
    Material* RenderEngine_DirectX11::createMaterialInternal()
    {
        return nullptr;
    }
    RenderTarget* RenderEngine_DirectX11::createRenderTargetInternal()
    {
        return nullptr;
    }
}

#endif
