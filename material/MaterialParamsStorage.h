// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "ShaderUniformInfo.h"
#include "jutils/jmap.h"
#include "jutils/jstringID.h"

namespace JumaRenderEngine
{
    class MaterialParamsStorage final
    {
    public:
        MaterialParamsStorage() = default;
        ~MaterialParamsStorage();

        template<ShaderUniformType Type>
        bool setValue(const jstringID& name, const typename ShaderUniformInfo<Type>::value_type& value)
        {
            return (name != jstringID_NONE) && this->setValueInternal<Type>(name, value);
        }
        bool setDefaultValue(const jstringID& name, ShaderUniformType type);
        bool removeValue(const jstringID& name, ShaderUniformType type);

        template<ShaderUniformType Type>
        bool getValue(const jstringID& name, typename ShaderUniformInfo<Type>::value_type& outValue) const
        {
            const typename ShaderUniformInfo<Type>::value_type* value = findValue<Type>(name);
            if (value == nullptr)
            {
                return false;
            }
            outValue = *value;
            return true;
        }
        bool contains(const jstringID& name, ShaderUniformType type) const;
        
        void clear();

    private:

        template<ShaderUniformType Type>
        using material_params_map = jmap<jstringID, typename ShaderUniformInfo<Type>::value_type>;

        material_params_map<ShaderUniformType::Float> m_MaterialParams_Float;
        material_params_map<ShaderUniformType::Vec4> m_MaterialParams_Vec4;
        material_params_map<ShaderUniformType::Mat4> m_MaterialParams_Mat4;
        material_params_map<ShaderUniformType::Texture> m_MaterialParams_Texture;


        template<ShaderUniformType Type>
        bool setValueInternal(const jstringID& name, const typename ShaderUniformInfo<Type>::value_type& value) { return false; }
        template<>
        bool setValueInternal<ShaderUniformType::Float>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Float>::value_type& value)
        {
            m_MaterialParams_Float[name] = value;
            return true;
        }
        template<>
        bool setValueInternal<ShaderUniformType::Vec4>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Vec4>::value_type& value)
        {
            m_MaterialParams_Vec4[name] = value;
            return true;
        }
        template<>
        bool setValueInternal<ShaderUniformType::Mat4>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Mat4>::value_type& value)
        {
            m_MaterialParams_Mat4[name] = value;
            return true;
        }
        template<>
        bool setValueInternal<ShaderUniformType::Texture>(const jstringID& name, const ShaderUniformInfo<ShaderUniformType::Texture>::value_type& value)
        {
            m_MaterialParams_Texture[name] = value;
            return true;
        }

        template<ShaderUniformType Type>
        const typename ShaderUniformInfo<Type>::value_type* findValue(const jstringID& name) const { return nullptr; }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Float>::value_type* findValue<ShaderUniformType::Float>(const jstringID& name) const { return m_MaterialParams_Float.find(name); }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Vec4>::value_type* findValue<ShaderUniformType::Vec4>(const jstringID& name) const { return m_MaterialParams_Vec4.find(name); }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Mat4>::value_type* findValue<ShaderUniformType::Mat4>(const jstringID& name) const { return m_MaterialParams_Mat4.find(name); }
        template<>
        const ShaderUniformInfo<ShaderUniformType::Texture>::value_type* findValue<ShaderUniformType::Texture>(const jstringID& name) const { return m_MaterialParams_Texture.find(name); }
    };
}
