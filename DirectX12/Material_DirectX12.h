// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_DIRECTX12)

#include "renderEngine/Material.h"

namespace JumaRenderEngine
{
    class Material_DirectX12 : public Material
    {
    public:
        Material_DirectX12() = default;
        virtual ~Material_DirectX12() override = default;
    };
}

#endif
