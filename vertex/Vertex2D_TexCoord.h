// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "VertexInfo.h"
#include "jutils/math/vector2.h"

namespace JumaRenderEngine
{
    struct Vertex2D_TexCoord
    {
        math::vector2 position;
        math::vector2 textureCoords;
    };

    template<>
    struct VertexInfo<Vertex2D_TexCoord> : std::true_type
    {
        JUMARENDERENGINE_VERTEX_TYPE(Vertex2D_TexCoord)

        static jarray<VertexComponentDescription> getVertexComponents()
        {
            return {
                { JSTR("position"), VertexComponentType::Vec2, 0, offsetof(VertexType, position) },
                { JSTR("textureCoords"), VertexComponentType::Vec2, 1, offsetof(VertexType, textureCoords) }
            };
        }
    };
}
