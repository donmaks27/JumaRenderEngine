// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer.h"

#include "vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    bool VertexBuffer::init(const VertexBufferData* verticesData)
    {
        m_VertexTypeName = verticesData->getVertexTypeName();
        if (!initInternal(verticesData))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize vertex buffer"));
            return false;
        }
        return true;
    }
}
