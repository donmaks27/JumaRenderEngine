// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include "renderEngine/juma_render_engine_core.h"

#if defined(JUMARENDERENGINE_INCLUDE_RENDER_API_VULKAN)

namespace JumaRenderEngine
{
    enum class VulkanQueueType : uint8
    {
        Graphics, Transfer
    };
}

#endif
