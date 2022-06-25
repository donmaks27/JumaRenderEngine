// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "VertexBuffer_OpenGL.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include <GL/glew.h>

#include "Material_OpenGL.h"
#include "renderEngine/RenderEngine.h"
#include "renderEngine/RenderOptions.h"
#include "renderEngine/RenderTarget.h"
#include "renderEngine/vertex/VertexBufferData.h"
#include "renderEngine/window/OpenGL/WindowController_OpenGL.h"

namespace JumaRenderEngine
{
    VertexBuffer_OpenGL::~VertexBuffer_OpenGL()
    {
        clearOpenGL();
    }

    bool VertexBuffer_OpenGL::initInternal(VertexBufferData* verticesData)
    {
        const uint32 vertexCount = verticesData->getVertexCount();
        if (vertexCount == 0)
        {
            JUMA_RENDER_LOG(error, JSTR("Empty vertex buffer data"));
            return false;
        }
        const VertexDescription* vertexDescription = getRenderEngine()->findVertexType(getVertexTypeName());

        uint32 verticesVBO = 0, indicesVBO = 0;
        glGenBuffers(1, &verticesVBO);
        glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
        glBufferData(
            GL_ARRAY_BUFFER, static_cast<int32>(vertexDescription->size * vertexCount), verticesData->getVertices(), GL_STATIC_DRAW
        );

        const uint32 indexCount = verticesData->getIndexCount();
        if (indexCount > 0)
        {
            glGenBuffers(1, &indicesVBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesVBO);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                static_cast<int32>(sizeof(uint32) * indexCount), 
                verticesData->getIndices(), 
                GL_STATIC_DRAW
            );
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        m_VerticesBufferIndex = verticesVBO;
        m_IndicesBufferIndex = indicesVBO;
        m_RenderElementsCount = static_cast<int32>(indexCount > 0 ? indexCount : vertexCount);
        return true;
    }

    void VertexBuffer_OpenGL::clearOpenGL()
    {
        if (!m_VertexArrayIndices.isEmpty())
        {
            WindowController_OpenGL* windowController = getRenderEngine()->getWindowController<WindowController_OpenGL>();
            const window_id prevWindowID = windowController->getActiveWindowID();
            for (const auto& VAO : m_VertexArrayIndices)
            {
                if (VAO.value != 0)
                {
                    windowController->setActiveWindowID(VAO.key);
                    glDeleteVertexArrays(1, &VAO.value);
                }
            }
            windowController->setActiveWindowID(prevWindowID);
            m_VertexArrayIndices.clear();
        }
        
        if (m_IndicesBufferIndex != 0)
        {
            glDeleteBuffers(1, &m_IndicesBufferIndex);
            m_IndicesBufferIndex = 0;
        }
        if (m_VerticesBufferIndex != 0)
        {
            glDeleteBuffers(1, &m_VerticesBufferIndex);
            m_VerticesBufferIndex = 0;
        }
        m_RenderElementsCount = 0;
    }

    void VertexBuffer_OpenGL::render(const RenderOptions* renderOptions, Material* material)
    {
        if ((renderOptions == nullptr) || (material == nullptr))
        {
            return;
        }

        Material_OpenGL* materialOpenGL = dynamic_cast<Material_OpenGL*>(material);

        const window_id windowID = renderOptions->renderTarget->getWindowID();
        const uint32 VAO = getVerticesVAO(windowID);
        if ((VAO != 0) && materialOpenGL->bindMaterial())
        {
            glBindVertexArray(VAO);
            if (m_IndicesBufferIndex != 0)
            {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndicesBufferIndex);
                glDrawElements(GL_TRIANGLES, m_RenderElementsCount, GL_UNSIGNED_INT, nullptr);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            }
            else
            {
                glDrawArrays(GL_TRIANGLES, 0, m_RenderElementsCount);
            }
            glBindVertexArray(0);

            materialOpenGL->unbindMaterial();
        }
    }
    uint32 VertexBuffer_OpenGL::getVerticesVAO(const window_id windowID)
    {
        const uint32* VAOPtr = m_VertexArrayIndices.find(windowID);
        if (VAOPtr != nullptr)
        {
            return *VAOPtr;
        }

        const uint32 VAO = createVerticesVAO();
        return VAO != 0 ? (m_VertexArrayIndices[windowID] = VAO) : 0;
    }
    uint32 VertexBuffer_OpenGL::createVerticesVAO() const
    {
        const VertexDescription* vertexDescription = getRenderEngine()->findVertexType(getVertexTypeName());
        if (vertexDescription == nullptr)
        {
            return 0;
        }

        uint32 VAO = 0;
        glGenVertexArrays(1, &VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VerticesBufferIndex);
        glBindVertexArray(VAO);
        for (int32 index = 0; index < vertexDescription->components.getSize(); index++)
        {
            const VertexComponentDescription& componentDescriprion = vertexDescription->components[index];

            GLenum componentType;
            GLint componentSize;
            switch (componentDescriprion.type)
            {
            case VertexComponentType::Float: 
                componentType = GL_FLOAT;
                componentSize = 1;
                break;
            case VertexComponentType::Vec2: 
                componentType = GL_FLOAT;
                componentSize = 2;
                break;
            case VertexComponentType::Vec3: 
                componentType = GL_FLOAT;
                componentSize = 3;
                break;
            case VertexComponentType::Vec4: 
                componentType = GL_FLOAT;
                componentSize = 4;
                break;
            default: continue;
            }
            
            glVertexAttribPointer(
                componentDescriprion.shaderLocation, componentSize, componentType, GL_FALSE, 
                static_cast<GLsizei>(vertexDescription->size), (const void*)static_cast<std::uintptr_t>(componentDescriprion.offset)
            );
            glEnableVertexAttribArray(componentDescriprion.shaderLocation);
        }
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return VAO;
    }
}

#endif
