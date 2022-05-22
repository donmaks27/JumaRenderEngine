// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer.h"

#include "vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    VertexBuffer::~VertexBuffer()
    {
        clearData();
    }

    bool VertexBuffer::init(const VertexBufferData* verticesData)
    {
        m_VertexTypeName = verticesData->getVertexTypeName();
        if (!initInternal(verticesData))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize vertex buffer"));
            clearData();
            return false;
        }
        return true;
    }

    void VertexBuffer::clearData()
    {
        m_VertexTypeName = jstringID_NONE;
    }
}
