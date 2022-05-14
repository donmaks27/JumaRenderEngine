// Copyright 2022 Leonov Maksim. All Rights Reserved.

#pragma once

#ifndef JDEBUG
    #define JUTILS_LOG_DISABLED
#endif
#include "jutils/jlog.h"

#define JUMA_RENDER_LOG(type, message) JUTILS_LOG_WRITE(type, message)
#define JUMA_RENDER_ERROR_LOG(errorCode, message) JUMA_RENDER_LOG(error, jutils::jstring(message) + JSTR(". Code ") + TO_JSTR(errorCode))

namespace JumaRenderEngine
{
    using namespace jutils;
}
namespace JRE = JumaRenderEngine;
