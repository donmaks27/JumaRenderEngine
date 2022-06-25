// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "VertexDescription.h"

namespace JumaRenderEngine
{
    template<typename T>
    struct VertexInfo : std::false_type
    {
        static const jstringID& getVertexTypeName()
        {
            static jstringID vertexName = jstringID_NONE;
            return vertexName;
        }
        static uint32 getVertexSize() { return 0; }

        static jarray<VertexComponentDescription> getVertexComponents() { return {}; }

        static void rotateTextureCoords(T& vertex) {}
    };

    template<typename T>
    constexpr bool is_vertex_type = VertexInfo<T>::value;
}

#define JUMARENDERENGINE_VERTEX_TYPE(VertexName)                \
    using VertexType = VertexName;                              \
    static const jstringID& getVertexTypeName()                 \
    {                                                           \
        static jstringID vertexName = #VertexName;              \
        return vertexName;                                      \
    }                                                           \
    static uint32 getVertexSize() { return sizeof(VertexName); }