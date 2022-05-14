// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "jutils/jarray.h"
#include "jutils/jstringID.h"

namespace JumaRenderEngine
{
    enum class VertexComponentType : uint8
    {
        Float, Vec2, Vec3, Vec4
    };
    struct VertexComponentDescription
    {
        jstringID name = jstringID_NONE;
        VertexComponentType type = VertexComponentType::Float;

        uint32 shaderLocation = 0;
        uint32 offset = 0;
    };
    struct VertexDescription
    {
        uint32 size = 0;
        jarray<VertexComponentDescription> components;
    };
}
