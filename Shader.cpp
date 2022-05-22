// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Shader.h"

namespace JumaRenderEngine
{
    Shader::~Shader()
    {
        clearData();
    }

    bool Shader::init(const jmap<ShaderStageFlags, jstring>& fileNames, jmap<jstringID, ShaderUniform> uniforms)
    {
        m_ShaderUniforms = std::move(uniforms);
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
        m_ShaderUniforms.clear();
    }
}
