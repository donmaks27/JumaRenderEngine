// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "RenderEngineContextObject.h"

#include "jutils/jmap.h"
#include "jutils/jstringID.h"
#include "material/ShaderUniform.h"

namespace JumaRenderEngine
{
    class Shader : public RenderEngineContextObjectBase
    {
        friend RenderEngine;

    public:
        Shader() = default;
        virtual ~Shader() override;

        const jmap<jstringID, ShaderUniform>& getUniforms() const { return m_ShaderUniforms; }

    protected:

        bool init(const jmap<ShaderStageFlags, jstring>& fileNames, jmap<jstringID, ShaderUniform> uniforms = {});

        virtual bool initInternal(const jmap<ShaderStageFlags, jstring>& fileNames) = 0;

    private:

        jmap<jstringID, ShaderUniform> m_ShaderUniforms;


        void clearData();
    };
}
