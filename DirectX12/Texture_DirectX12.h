// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/Texture.h"

namespace JumaRenderEngine
{
    class Texture_DirectX12 : public Texture
    {
    public:
        Texture_DirectX12() = default;
        virtual ~Texture_DirectX12() override = default;
    };
}

#endif
