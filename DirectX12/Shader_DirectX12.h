// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/Shader.h"

namespace JumaRenderEngine
{
    class Shader_DirectX12 : public Shader
    {
    public:
        Shader_DirectX12() = default;
        virtual ~Shader_DirectX12() override = default;
    };
}

#endif
