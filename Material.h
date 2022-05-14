// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "RenderEngineContextObject.h"

#include "material/MaterialParamsStorage.h"

namespace JumaRenderEngine
{
    class Shader;

    class Material : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        Material() = default;
        virtual ~Material() override;

        Shader* getShader() const { return m_Shader; }
        const MaterialParamsStorage& getMaterialParams() const { return m_MaterialParams; }

        template<ShaderUniformType Type>
        bool setParamValue(const jstringID& name, const typename ShaderUniformInfo<Type>::value_type& value)
        {
            return checkParamType(name, Type) && m_MaterialParams.setValue<Type>(name, value);
        }
        bool resetParamValue(const jstringID& name);
        template<ShaderUniformType Type>
        bool getValue(const jstringID& name, typename ShaderUniformInfo<Type>::value_type& outValue) const
        {
            return checkParamType(name, Type) && m_MaterialParams.getValue<Type>(name, outValue);
        }

    protected:
        
        bool init(Shader* shader);

        virtual bool initInternal();

    private:

        Shader* m_Shader = nullptr;
        MaterialParamsStorage m_MaterialParams;


        void clearData();

        bool checkParamType(const jstringID& name, ShaderUniformType type) const;
    };
}
