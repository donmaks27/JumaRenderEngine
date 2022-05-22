// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "RenderEngineContextObject.h"

#include "jutils/jstringID.h"

namespace JumaRenderEngine
{
    struct RenderOptions;
    class Material;
    class VertexBufferData;

    class VertexBuffer : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        VertexBuffer() = default;
        virtual ~VertexBuffer() override;

        const jstringID& getVertexTypeName() const { return m_VertexTypeName; }

        virtual void render(const RenderOptions* renderOptions, Material* material) = 0;

    protected:

        bool init(const VertexBufferData* verticesData);

        virtual bool initInternal(const VertexBufferData* verticesData) = 0;

    private:

        jstringID m_VertexTypeName = jstringID_NONE;


        void clearData();
    };
}
