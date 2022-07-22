// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "RenderEngineContextObject.h"

#include "jutils/jmap.h"
#include "jutils/jset.h"
#include "jutils/jstringID.h"
#include "material/ShaderUniform.h"

namespace JumaRenderEngine
{
    struct ShaderUniformBufferDescription
    {
        uint32 size = 0;
        uint8 shaderStages = 0;
    };

    class Shader : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        Shader() = default;
        virtual ~Shader() override;

        const jset<jstringID>& getRequiredVertexComponents() const { return m_VertexComponents; }

        const jmap<jstringID, ShaderUniform>& getUniforms() const { return m_ShaderUniforms; }
        const jmap<uint32, ShaderUniformBufferDescription>& getUniformBufferDescriptions() const { return m_CachedUniformBufferDescriptions; }

    protected:

        bool init(const jmap<ShaderStageFlags, jstring>& fileNames, jset<jstringID> vertexComponents, jmap<jstringID, ShaderUniform> uniforms = {});

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) = 0;

    private:

        jset<jstringID> m_VertexComponents;
        jmap<jstringID, ShaderUniform> m_ShaderUniforms;
        jmap<uint32, ShaderUniformBufferDescription> m_CachedUniformBufferDescriptions;


        void clearData();
    };
}
