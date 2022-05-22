// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Material.h"

#include "Shader.h"

namespace JumaRenderEngine
{
    Material::~Material()
    {
        clearData();
    }

    bool Material::init(Shader* shader)
    {
        if (shader == nullptr)
        {
            JUMA_RENDER_LOG(error, JSTR("Invalid shader"));
            return false;
        }

        m_Shader = shader;
        if (!initInternal())
        {
            JUMA_RENDER_LOG(error, JSTR("Failed to initizlie material"));
            clearData();
            return false;
        }
        return true;
    }
    bool Material::initInternal()
    {
        for (const auto& uniform : m_Shader->getUniforms())
        {
            m_MaterialParams.setDefaultValue(uniform.key, uniform.value.type);
        }
        return true;
    }

    void Material::clearData()
    {
        m_MaterialParams.clear();
        m_Shader = nullptr;
    }

    bool Material::checkParamType(const jstringID& name, const ShaderUniformType type) const
    {
        const ShaderUniform* uniform = m_Shader->getUniforms().find(name);
        return (uniform != nullptr) && (uniform->type == type);
    }
    bool Material::resetParamValue(const jstringID& name)
    {
        const ShaderUniform* uniform = m_Shader->getUniforms().find(name);
        return (uniform != nullptr) && m_MaterialParams.setDefaultValue(name, uniform->type);
    }
}
