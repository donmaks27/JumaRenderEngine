// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "RenderAPI.h"
#include "RenderEngineContextObject.h"
#include "jutils/jmap.h"
#include "material/ShaderUniform.h"
#include "texture/TextureFormat.h"
#include "texture/TextureSamples.h"
#include "vertex/VertexDescription.h"
#include "window/WindowController.h"

namespace JumaRenderEngine
{
    class RenderPipeline;
    class Texture;
    class RenderTarget;
    class Material;
    class Shader;
    class VertexBuffer;
    class VertexBufferData;

    class RenderEngine
    {
    public:
        RenderEngine() = default;
        virtual ~RenderEngine();

        virtual RenderAPI getRenderAPI() const = 0;

        bool init(const jmap<window_id, WindowProperties>& windows);
        bool isValid() const { return m_Initialized; }
        void clear();

        WindowController* getWindowController() const { return m_WindowController; }
        template<typename T, TEMPLATE_ENABLE(is_base<WindowController, T>)>
        T* getWindowController() const { return dynamic_cast<T*>(this->getWindowController()); }

        template<typename T, TEMPLATE_ENABLE(is_base_and_not_abstract<RenderEngineContextObjectBase, T>)>
        T* createObject() { return this->registerObject(new T()); }
        template<typename T, TEMPLATE_ENABLE(is_base<RenderEngineContextObjectBase, T>)>
        T* registerObject(T* object)
        {
            this->registerObjectInternal(object);
            return object;
        }

        RenderPipeline* getRenderPipeline() const { return m_RenderPipeline; }
        template<typename T, TEMPLATE_ENABLE(is_base<RenderPipeline, T>)>
        T* getRenderPipeline() const { return dynamic_cast<T*>(getRenderPipeline()); }

        VertexBuffer* createVertexBuffer(const VertexBufferData* verticesData);
        const VertexDescription* findVertexType(const jstringID& vertexName) const { return m_RegisteredVertexTypes.find(vertexName); }

        Texture* createTexture(const math::uvector2& size, TextureFormat format, const uint8* data);

        Shader* createShader(const jmap<ShaderStageFlags, jstring>& fileNames, jmap<jstringID, ShaderUniform> uniforms = {});
        Material* createMaterial(Shader* shader);

        RenderTarget* createRenderTarget(window_id windowID, TextureSamples samples);
        RenderTarget* createRenderTarget(TextureFormat format, const math::uvector2& size, TextureSamples samples);

    protected:

        virtual bool initInternal(const jmap<window_id, WindowProperties>& windows);
        virtual void clearInternal() { clearData(); }

        void clearRenderAssets();
        void clearData();

        virtual WindowController* createWindowController() = 0;
        virtual VertexBuffer* createVertexBufferInternal() = 0;
        virtual Texture* createTextureInternal() = 0;
        virtual Shader* createShaderInternal() = 0;
        virtual Material* createMaterialInternal() = 0;
        virtual RenderTarget* createRenderTargetInternal() = 0;
        virtual RenderPipeline* createRenderPipelineInternal();

        virtual void onRegisteredVertexType(const jstringID& vertexName) {}

    private:

        bool m_Initialized = false;

        WindowController* m_WindowController = nullptr;
        RenderPipeline* m_RenderPipeline = nullptr;
        jmap<jstringID, VertexDescription> m_RegisteredVertexTypes;


        bool createRenderAssets();

        void registerObjectInternal(RenderEngineContextObjectBase* object);
        
        const VertexDescription* registerVertexType(const VertexBufferData* verticesData);
    };
}
