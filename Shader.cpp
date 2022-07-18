// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Shader.h"

#include "material/ShaderUniformInfo.h"

namespace JumaRenderEngine
{
    Shader::~Shader()
    {
        clearData();
    }

    bool Shader::init(const jmap<ShaderStageFlags, jstring>& fileNames, jset<jstringID> vertexComponents, jmap<jstringID, ShaderUniform> uniforms)
    {
        m_VertexComponents = std::move(vertexComponents);

        m_ShaderUniforms = std::move(uniforms);
        for (const auto& uniform : m_ShaderUniforms)
        {
            const uint32 size = GetShaderUniformValueSize(uniform.value.type);
            if (size == 0)
            {
                continue;
            }

            ShaderUniformBufferDescription& uniformBuffer = m_CachedUniformBufferDescriptions[uniform.value.shaderLocation];
            uniformBuffer.size = math::max(uniformBuffer.size, uniform.value.shaderBlockOffset + size);
            uniformBuffer.shaderStages |= uniform.value.shaderStages;
        }

        if (!initInternal(fileNames))
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initialize shader"));
            clearData();
            return false;
        }
        return true;
    }

    void Shader::clearData()
    {
        m_CachedUniformBufferDescriptions.clear();
        m_ShaderUniforms.clear();
        m_VertexComponents.clear();
    }
}
