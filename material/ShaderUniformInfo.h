// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#include "ShaderUniform.h"
#include "jutils/math/vector2.h"
#include "jutils/math/vector4.h"
#include "jutils/math/matrix4.h"

namespace JumaRenderEngine
{
    class TextureBase;

    template<ShaderUniformType Type>
    struct ShaderUniformInfo : std::false_type {};
    
    template<>
    struct ShaderUniformInfo<ShaderUniformType::Float> : std::true_type
    {
        using value_type = float;
    };
    template<>
    struct ShaderUniformInfo<ShaderUniformType::Vec2> : std::true_type
    {
        using value_type = math::vector2;
    };
    template<>
    struct ShaderUniformInfo<ShaderUniformType::Vec4> : std::true_type
    {
        using value_type = math::vector4;
    };
    template<>
    struct ShaderUniformInfo<ShaderUniformType::Mat4> : std::true_type
    {
        using value_type = math::matrix4;
    };
    template<>
    struct ShaderUniformInfo<ShaderUniformType::Texture> : std::true_type
    {
        using value_type = TextureBase*;
    };

    constexpr bool IsShaderUniformScalar(const ShaderUniformType type)
    {
        switch (type)
        {
        case ShaderUniformType::Float:
        case ShaderUniformType::Vec2:
        case ShaderUniformType::Vec4:
        case ShaderUniformType::Mat4:
            return true;

        default: ;
        }
        return false;
    }
    constexpr uint32 GetShaderUniformValueSize(const ShaderUniformType type)
    {
        switch (type)
        {
        case ShaderUniformType::Float: return sizeof(ShaderUniformInfo<ShaderUniformType::Float>::value_type);
        case ShaderUniformType::Vec2: return sizeof(ShaderUniformInfo<ShaderUniformType::Vec2>::value_type);
        case ShaderUniformType::Vec4: return sizeof(ShaderUniformInfo<ShaderUniformType::Vec4>::value_type);
        case ShaderUniformType::Mat4: return sizeof(ShaderUniformInfo<ShaderUniformType::Mat4>::value_type);
        default: ;
        }
        return 0;
    }
}
