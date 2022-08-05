// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#include <jutils/jlog.h>
#define JUMA_RENDER_LOG(type, ...) JUTILS_LOG(type, __VA_ARGS__)
#define JUMA_RENDER_ERROR_LOG(errorCode, ...) JUTILS_ERROR_LOG(errorCode, __VA_ARGS__)

namespace JumaRenderEngine
{
    using namespace jutils;
}
namespace JRE = JumaRenderEngine;
