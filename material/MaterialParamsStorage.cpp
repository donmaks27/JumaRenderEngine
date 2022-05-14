// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "MaterialParamsStorage.h"

namespace JumaRenderEngine
{
    MaterialParamsStorage::~MaterialParamsStorage()
    {
        clear();
    }

    bool MaterialParamsStorage::setDefaultValue(const jstringID& name, const ShaderUniformType type)
    {
        switch (type)
        {
        case ShaderUniformType::Vec4: return setValue<ShaderUniformType::Vec4>(name, math::vector4(0));
        case ShaderUniformType::Mat4: return setValue<ShaderUniformType::Mat4>(name, math::matrix4(1));
        case ShaderUniformType::Texture: return setValue<ShaderUniformType::Texture>(name, nullptr);
        default: ;
        }
        return false;
    }

    bool MaterialParamsStorage::removeValue(const jstringID& name, const ShaderUniformType type)
    {
        switch (type)
        {
        case ShaderUniformType::Vec4: return m_MaterialParams_Vec4.remove(name);
        case ShaderUniformType::Mat4: return m_MaterialParams_Mat4.remove(name);
        case ShaderUniformType::Texture: return m_MaterialParams_Texture.remove(name);
        default: ;
        }
        return false;
    }

    bool MaterialParamsStorage::contains(const jstringID& name, const ShaderUniformType type) const
    {
        switch (type)
        {
        case ShaderUniformType::Vec4: return m_MaterialParams_Vec4.contains(name);
        case ShaderUniformType::Mat4: return m_MaterialParams_Mat4.contains(name);
        case ShaderUniformType::Texture: return m_MaterialParams_Texture.contains(name);
        default: ;
        }
        return false;
    }

    void MaterialParamsStorage::clear()
    {
        m_MaterialParams_Texture.clear();
        m_MaterialParams_Mat4.clear();
        m_MaterialParams_Vec4.clear();
        m_MaterialParams_Float.clear();
    }
}
