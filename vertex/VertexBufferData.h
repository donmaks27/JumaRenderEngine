// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "VertexInfo.h"

namespace JumaRenderEngine
{
    class VertexBufferData
    {
    public:
        VertexBufferData() = default;
        virtual ~VertexBufferData() = default;

        virtual const jstringID& getVertexTypeName() const = 0;
        virtual VertexDescription getVertexDescription() const = 0;

        virtual const void* getVertices() const = 0;
        virtual uint32 getVertexCount() const = 0;

        const void* getIndices() const { return !vertexIndices.isEmpty() ? vertexIndices.getData() : nullptr; }
        uint32 getIndexCount() const { return static_cast<uint32>(vertexIndices.getSize()); }

        void setVertexIndices(jarray<uint32> data) { vertexIndices = std::move(data); }

        virtual void rotateTextureCoords() = 0;

    protected:

        jarray<uint32> vertexIndices;
    };

    template<typename T, TEMPLATE_ENABLE(is_vertex_type<T>)>
    class VertexBufferDataImpl final : public VertexBufferData
    {
    public:
        VertexBufferDataImpl() = default;
        virtual ~VertexBufferDataImpl() override = default;

        using VertexType = T;

        virtual const jstringID& getVertexTypeName() const override { return VertexInfo<VertexType>::getVertexTypeName(); }
        virtual VertexDescription getVertexDescription() const override { return { VertexInfo<VertexType>::getVertexSize(), VertexInfo<VertexType>::getVertexComponents() }; }

        virtual const void* getVertices() const override { return vertices.getData(); }
        virtual uint32 getVertexCount() const override { return static_cast<uint32>(vertices.getSize()); }
        
        void setVertices(jarray<VertexType> data) { vertices = std::move(data); }

        virtual void rotateTextureCoords() override
        {
            for (auto& vertex : vertices)
            {
                VertexInfo<VertexType>::rotateTextureCoords(vertex);
            }
        }

    private:

        jarray<VertexType> vertices;
    };
}
