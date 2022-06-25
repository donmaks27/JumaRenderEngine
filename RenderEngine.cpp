// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "RenderEngine.h"

#include "Material.h"
#include "RenderPipeline.h"
#include "RenderTarget.h"
#include "Shader.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "vertex/VertexBufferData.h"

namespace JumaRenderEngine
{
    RenderEngine::~RenderEngine()
    {
        clearData();
    }

    bool RenderEngine::init(const jmap<window_id, WindowProperties>& windows)
    {
        if (isValid())
        {
            JUMA_RENDER_LOG(warning, JSTR("Render engine already initialized"));
            return false;
        }
        if (windows.isEmpty())
        {
            JUMA_RENDER_LOG(error, JSTR("Empty list of windows, there must be at least one!"));
            return false;
        }

        WindowController* windowController = createWindowController();
        if (!windowController->initWindowController())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize window controller"));
            delete windowController;
            return false;
        }
        m_WindowController = windowController;
        if (!initInternal(windows))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize render engine"));
            clearInternal();
            return false;
        }
        m_Initialized = true;

        if (!createRenderAssets())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize render assets"));
            clear();
            return false;
        }
        return true;
    }
    bool RenderEngine::initInternal(const jmap<window_id, WindowProperties>& windows)
    {
        for (const auto& window : windows)
        {
            if (!m_WindowController->createWindow(window.key, window.value))
            {
                JUMA_RENDER_LOG(error, JSTR("Failed to create window ") + TO_JSTR(window.key));
                return false;
            }
        }
        return true;
    }

    bool RenderEngine::createRenderAssets()
    {
        RenderPipeline* renderPipeline = createRenderPipelineInternal();
        if (!renderPipeline->init())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to init render pipeline"));
            delete renderPipeline;
            return false;
        }
        m_RenderPipeline = renderPipeline;

        return true;
    }
    void RenderEngine::clearRenderAssets()
    {
        if (m_RenderPipeline != nullptr)
        {
            delete m_RenderPipeline;
            m_RenderPipeline = nullptr;
        }
    }

    void RenderEngine::clear()
    {
        if (m_Initialized)
        {
            clearInternal();
            m_Initialized = false;
        }
    }
    void RenderEngine::clearData()
    {
        if (m_WindowController != nullptr)
        {
            delete m_WindowController;
            m_WindowController = nullptr;
        }
        m_RegisteredVertexTypes.clear();
    }

    void RenderEngine::registerObjectInternal(RenderEngineContextObjectBase* object)
    {
        if (object != nullptr)
        {
            object->m_RenderEngine = this;
        }
    }
    RenderPipeline* RenderEngine::createRenderPipelineInternal()
    {
        return createObject<RenderPipeline>();
    }

    VertexBuffer* RenderEngine::createVertexBuffer(VertexBufferData* verticesData)
    {
        if (registerVertexType(verticesData) == nullptr)
        {
            return nullptr;
        }

        VertexBuffer* vertexBuffer = createVertexBufferInternal();
        if (!vertexBuffer->init(verticesData))
        {
            delete vertexBuffer;
            return nullptr;
        }
        return vertexBuffer;
    }
    const VertexDescription* RenderEngine::registerVertexType(const VertexBufferData* verticesData)
    {
        if (!isValid() || (verticesData == nullptr))
        {
            return nullptr;
        }

        const jstringID& vertexName = verticesData->getVertexTypeName();
        if (vertexName == jstringID_NONE)
        {
            return nullptr;
        }

        const VertexDescription* description = findVertexType(vertexName);
        if (description != nullptr)
        {
            return description;
        }

        description = &m_RegisteredVertexTypes.add(vertexName, verticesData->getVertexDescription());
        onRegisteredVertexType(vertexName);
        return description;
    }

    Texture* RenderEngine::createTexture(const math::uvector2& size, const TextureFormat format, const uint8* data)
    {
        Texture* texture = createTextureInternal();
        if (!texture->init(size, format, data))
        {
            delete texture;
            return nullptr;
        }
        return texture;
    }

    Shader* RenderEngine::createShader(const jmap<ShaderStageFlags, jstring>& fileNames, jmap<jstringID, ShaderUniform> uniforms)
    {
        Shader* shader = createShaderInternal();
        if (!shader->init(fileNames, std::move(uniforms)))
        {
            delete shader;
            return nullptr;
        }
        return shader;
    }
    Material* RenderEngine::createMaterial(Shader* shader)
    {
        Material* material = createMaterialInternal();
        if (!material->init(shader))
        {
            delete material;
            return nullptr;
        }
        return material;
    }

    RenderTarget* RenderEngine::createWindowRenderTarget(const window_id windowID, const TextureSamples samples)
    {
        RenderTarget* renderTarget = createRenderTargetInternal();
        if (!renderTarget->init(windowID, samples))
        {
            delete renderTarget;
            return nullptr;
        }
        return renderTarget;
    }
    RenderTarget* RenderEngine::createRenderTarget(const TextureFormat format, const math::uvector2& size, const TextureSamples samples)
    {
        RenderTarget* renderTarget = createRenderTargetInternal();
        if (!renderTarget->init(format, size, samples))
        {
            delete renderTarget;
            return nullptr;
        }
        return renderTarget;
    }
}
