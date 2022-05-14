// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "VertexInfo.h"
#include "jutils/math/vector2.h"

namespace JumaRenderEngine
{
    struct Vertex2D
    {
        math::vector2 position;
    };

    template<>
    struct VertexInfo<Vertex2D> : std::true_type
    {
        JUMARENDERENGINE_VERTEX_TYPE(Vertex2D)

        static jarray<VertexComponentDescription> getVertexComponents()
        {
            return {{ JSTR("position"), VertexComponentType::Vec2, 0, offsetof(VertexType, position) }};
        }
    };
}
