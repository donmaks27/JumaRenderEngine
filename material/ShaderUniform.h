// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

namespace JumaRenderEngine
{
    enum class ShaderUniformType : uint8
    {
        Float,
        Vec2,
        Vec4,
        Mat4,
        Texture
    };
    enum ShaderStageFlags : uint8
    {
        SHADER_STAGE_VERTEX   = 0b00000001,
        SHADER_STAGE_FRAGMENT = 0b00000010
    };
    struct ShaderUniform
    {
        ShaderUniformType type = ShaderUniformType::Mat4;

        uint8 shaderStages = 0;
        uint32 shaderLocation = 0;
        uint32 shaderBlockOffset = 0;
    };
}
