// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"
#include "TextureBase.h"

#include "texture/TextureFormat.h"

namespace JumaRenderEngine
{
    class Texture : public TextureBase
    {
        friend RenderEngine;

    public:
        Texture() = default;
        virtual ~Texture() override = default;

    protected:

        virtual bool initInternal(const math::uvector2& size, TextureFormat format, const uint8* data) = 0;

    private:

        bool init(const math::uvector2& size, TextureFormat format, const uint8* data);
    };
}
