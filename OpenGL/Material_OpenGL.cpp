// Copyright 2022 Leonov Maksim. All Rights Reserved.

#include "Material_OpenGL.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_OPENGL)

#include <GL/glew.h>

#include "RenderTarget_OpenGL.h"
#include "Shader_OpenGL.h"
#include "Texture_OpenGL.h"

namespace JumaRenderEngine
{
    Material_OpenGL::~Material_OpenGL()
    {
        clearOpenGL();
    }

    bool Material_OpenGL::initInternal()
    {
        if (!Super::initInternal())
        {
            return false;
        }

        const Shader_OpenGL* shader = dynamic_cast<const Shader_OpenGL*>(getShader());
        const jmap<uint32, uint32>& uniformBufferSizes = shader->getUniformBufferSizes();
        if (!uniformBufferSizes.isEmpty())
        {
            jarray<uint32> uniformBuffers(uniformBufferSizes.getSize(), 0);
            glGenBuffers(uniformBuffers.getSize(), uniformBuffers.getData());
            for (const auto& bufferSize : uniformBufferSizes)
            {
                const uint32 bufferIndex = m_UniformBufferIndices[bufferSize.key] = uniformBuffers.getLast();
                uniformBuffers.removeLast();

                glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);
                glBufferData(GL_UNIFORM_BUFFER, bufferSize.value, nullptr, GL_DYNAMIC_DRAW);
            }
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        return true;
    }

    void Material_OpenGL::clearOpenGL()
    {
        for (const auto& buffer : m_UniformBufferIndices)
        {
            glDeleteBuffers(1, &buffer.value);
        }
        m_UniformBufferIndices.clear();
    }

    bool Material_OpenGL::bindMaterial()
    {
        const Shader_OpenGL* shader = dynamic_cast<const Shader_OpenGL*>(getShader());
        if ((shader == nullptr) || !shader->activateShader())
        {
            return false;
        }

        const MaterialParamsStorage& materialParams = getMaterialParams();
        for (const auto& uniform : shader->getUniforms())
        {
            switch (uniform.value.type)
            {
            case ShaderUniformType::Float:
                {
                    ShaderUniformInfo<ShaderUniformType::Float>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Float>(uniform.key, value))
                    {
                        glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.value.shaderLocation]);
                        glBufferSubData(GL_UNIFORM_BUFFER, uniform.value.shaderBlockOffset, sizeof(value), &value);
                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    }
                }
                break;
            case ShaderUniformType::Vec4:
                {
                    ShaderUniformInfo<ShaderUniformType::Vec4>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Vec4>(uniform.key, value))
                    {
                        glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.value.shaderLocation]);
                        glBufferSubData(GL_UNIFORM_BUFFER, uniform.value.shaderBlockOffset, sizeof(value), &value[0]);
                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    }
                }
                break;
            case ShaderUniformType::Mat4:
                {
                    ShaderUniformInfo<ShaderUniformType::Mat4>::value_type value;
                    if (materialParams.getValue<ShaderUniformType::Mat4>(uniform.key, value))
                    {
                        glBindBuffer(GL_UNIFORM_BUFFER, m_UniformBufferIndices[uniform.value.shaderLocation]);
                        glBufferSubData(GL_UNIFORM_BUFFER, uniform.value.shaderBlockOffset, sizeof(value), &value[0][0]);
                        glBindBuffer(GL_UNIFORM_BUFFER, 0);
                    }
                }
                break;

            case ShaderUniformType::Texture:
                {
                    ShaderUniformInfo<ShaderUniformType::Texture>::value_type value = nullptr;
                    materialParams.getValue<ShaderUniformType::Texture>(uniform.key, value);

                    const Texture_OpenGL* texture = dynamic_cast<Texture_OpenGL*>(value);
                    if (texture != nullptr)
                    {
                        texture->bindTexture(uniform.value.shaderLocation);
                    }
                    else
                    {
                        const RenderTarget_OpenGL* renderTarget = dynamic_cast<RenderTarget_OpenGL*>(value);
                        if (renderTarget != nullptr)
                        {
                            renderTarget->bindResultTexture(uniform.value.shaderLocation);
                        }
                    }
                }
                break;

            default: ;
            }
        }
        
        for (const auto& uniformBuffer : m_UniformBufferIndices)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer.key, uniformBuffer.value);
        }

        return true;
    }

    void Material_OpenGL::unbindMaterial()
    {
        for (const auto& uniformBuffer : m_UniformBufferIndices)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, uniformBuffer.key, 0);
        }
        for (const auto& uniform : getShader()->getUniforms())
        {
            if (uniform.value.type == ShaderUniformType::Texture)
            {
                Texture_OpenGL::unbindTexture(uniform.value.shaderLocation);
            }
        }
        Shader_OpenGL::deactivateAnyShader();
    }
}

#endif
